#include "mwm.hpp"
#include "classes.hpp"

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define callHook(hook) if (hook != nullptr) hook(*this)
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define CLEANMASK(mask)         (mask & ~(wm.numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))

extern std::vector<Key> keys;
extern std::vector<Button> buttons;
extern const int borderWidth;
extern std::map<const char*, const char*> colors;
typedef void (*handlerFunction)(XEvent* event);
extern void startup();

Cur::Cur(int shape) {
	cursor = XCreateFontCursor(wm.display, shape);
}

void sigchld(int unused) {
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		std::cerr << "can't install SIGCHLD handler:" << std::endl;
	while (0 < waitpid(-1, NULL, WNOHANG));
}


mwm::mwm() {
	callHook(hookPreeInit);

	sigchld(0);

	Atom utf8string;
	XSetWindowAttributes windowAttributes;

	sigchld(0);

	// Setup the locale and display
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		std::cerr << "No locale support detected!" << std::endl;
	if (!(display = XOpenDisplay(NULL)))
		std::cerr << "Cannot open display!" << std::endl;

	// Check if an other window manager is running
	otherWMRunning(display);

	screen = DefaultScreen(display);
	displayWidth = DisplayWidth(display, screen);
	displayHeight = DisplayHeight(display, screen);
	root = RootWindow(display, screen);

	drawable = XCreatePixmap(wm.display, wm.root, wm.displayWidth, wm.displayHeight, DefaultDepth(wm.display, wm.screen));
	gc = XCreateGC(wm.display, wm.root, 0, NULL);
	XSetLineAttributes(wm.display, gc, 1, LineSolid, CapButt, JoinMiter);

	// Setup the atoms
	utf8string = XInternAtom(display, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(display, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(display, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(display, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(display, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(display, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(display, "_NET_CLIENT_LIST", False);

	// Setup the cursors
	cursor[CurNormal] = new Cur(XC_left_ptr);
	cursor[CurResize] = new Cur(XC_sizing);
	cursor[CurMove] = new Cur(XC_fleur);

	// Setup the colors
	initColors(colors);

	// Support for checkwm
	wmcheckwin = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(display, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(display, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "mwm", 3);
	XChangeProperty(display, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);

	// EWMH support per view
	XChangeProperty(display, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	XDeleteProperty(display, root, netatom[NetClientList]);

	// Set the cursor and event mask
	windowAttributes.cursor = cursor[CurNormal]->cursor;
	windowAttributes.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;

	XChangeWindowAttributes(display, root, CWEventMask|CWCursor, &windowAttributes);
	XSelectInput(display, root, windowAttributes.event_mask);

	grabKeys();

	callHook(hookPostInit);

	scan();
	startup();
}

mwm::~mwm() {
	XUngrabKey(display, AnyKey, AnyModifier, root);
	XDestroyWindow(display, wmcheckwin);
	XSync(display, false);
	XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(display, root, netatom[NetActiveWindow]);

	XCloseDisplay(display);
}

void mwm::initColors(std::map<const char*, const char*> &colors) {
	XftColor tmp;
	for (auto& c : colors) {
		XftColorAllocName(
			display,
			DefaultVisual(display, screen),
			DefaultColormap(display, screen),
			c.first,
			//&xColors[c.first]
			&tmp
		);
		xColors.insert_or_assign(c.first, std::move(tmp));
	}
}

void mwm::updateNumlockMaks() {
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(display);
	for (i = 0; i < 8; ++i)
		for (j = 0; j < modmap->max_keypermod; ++j)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(display, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void mwm::manage(Window win, XWindowAttributes *windoAttributes) {
	Client *client, *t = NULL;
	Window trans = None;
	XWindowChanges windowChanges;

	windowChanges.border_width = borderWidth;
	XConfigureWindow(wm.display, win, CWBorderWidth, &windowChanges);
	XSetWindowBorder(wm.display, win, xColors["border"].pixel);
	configure(client);
	XSelectInput(display, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);

	grabButtons(client, false);

	XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char*) &(client->win), 1);
	XMoveResizeWindow(display, client->win, client->x + 2 * displayWidth, client->y, client->w, client->h);

}
void mwm::unmanage(Client *client, bool destroyed) {
	Monitor *monitor = client->monitor;
	XWindowChanges windowChanges;

	delete client;
}

void mwm::scan() {
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(wm.display, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(wm.display, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(wm.display, wins[i], &d1))
				continue;
			//if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
			if (wa.map_state == IsViewable)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(wm.display, wins[i], &wa))
				continue;
			if (XGetTransientForHint(wm.display, wins[i], &d1)
			//&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
			&& (wa.map_state == IsViewable))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void mwm::grabButtons(Client *client, bool focused) {
	updateNumlockMaks();

	unsigned int i, j;
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
	XUngrabButton(display, AnyButton, AnyModifier, client->win);
	if (!focused)
		XGrabButton(display, AnyButton, AnyModifier, client->win, False,
			BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	for (i = 0; i < buttons.size(); i++)
		if (buttons[i].click == ClkClientWin)
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabButton(display, buttons[i].button,
					buttons[i].mask | modifiers[j],
					client->win, False, BUTTONMASK,
					GrabModeAsync, GrabModeSync, None, None);
}

void mwm::grabKeys() {
	updateNumlockMaks();

	unsigned int i, j;
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
	KeyCode code;

	XUngrabKey(display, AnyKey, AnyModifier, root);
	for (i = 0; i < keys.size(); i++)
		if ((code = XKeysymToKeycode(display, keys[i].keysym)))
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(display, code, keys[i].mod | modifiers[j], root,
					True, GrabModeAsync, GrabModeAsync);
}

void mwm::configure(Client *client) {
	XConfigureEvent configureEvent;

	configureEvent.type = ConfigureNotify;
	configureEvent.display = wm.display;
	configureEvent.event = client->win;
	configureEvent.window = client->win;
	configureEvent.x = client->x;
	configureEvent.y = client->y;
	configureEvent.width = client->w;
	configureEvent.height = client->h;
	configureEvent.border_width = borderWidth;
	configureEvent.above = None;
	configureEvent.override_redirect = false;

	XSendEvent(wm.display, client->win, false, StructureNotifyMask, (XEvent*)&configureEvent);
}

void buttonPress(XEvent* event) {
	std::cout << "buttonPress" << std::endl;
}
void clientMessage(XEvent* event) {
	std::cout << "clientMessage" << std::endl;
}
void configureRequest(XEvent* event) {
	std::cout << "configureRequest" << std::endl;
	Client *client;
	Monitor* monitor;
	XConfigureRequestEvent *ev = &event->xconfigurerequest;
	XWindowChanges windowChanges;

	for (auto& ws : wm.Workspaces)
		ws->tile(*ws);

	XSync(wm.display, false);
}
void configureNotify(XEvent* event) {
	std::cout << "configureNotify" << std::endl;
}
void destroyNotify(XEvent* event) {
	std::cout << "destroyNotify" << std::endl;
	Client *client;
	XDestroyWindowEvent *ev = &event->xdestroywindow;


}
void enterNotify(XEvent* event) {
	std::cout << "enterNotify" << std::endl;
}
void expose(XEvent* event) {
	std::cout << "expose" << std::endl;
}
void focusIn(XEvent* event) {
	std::cout << "focusIn" << std::endl;
}
void keyPress(XEvent* event) {
	std::cout << "keyPress" << std::endl;
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &event->xkey;
	keysym = XKeycodeToKeysym(wm.display, (KeyCode)ev->keycode, 0);
	for (i = 0; i < keys.size(); i++)
		if (keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(keys[i].args);
}
void mappingNotify(XEvent* event) {
	std::cout << "mappingNotify" << std::endl;
}
void mapRequest(XEvent* event) {
	std::cout << "mapRequest" << std::endl;
	
	static XWindowAttributes windowAttributes;
	XMapRequestEvent *ev = &event->xmaprequest;

	if (!XGetWindowAttributes(wm.display, ev->window, &windowAttributes))
		return;
	if (windowAttributes.override_redirect)
		return;
	wm.manage(ev->window, &windowAttributes);
}
void motionNotify(XEvent* event) {
	std::cout << "motionNotify" << std::endl;
}
void propertyNotify(XEvent* event) {
	std::cout << "propertyNotify" << std::endl;

	Client* client;
	Window trans;
	XPropertyEvent *ev = &event->xproperty;
}
void unmapNotify(XEvent* event) {
	std::cout << "unmapNotify" << std::endl;
}

std::map<int, handlerFunction> handler = {
	{ ButtonPress, buttonPress },
	{ ClientMessage, clientMessage },
	{ ConfigureRequest, configureRequest },
	{ ConfigureNotify, configureNotify },
	{ DestroyNotify, destroyNotify },
	{ EnterNotify, enterNotify },
	{ Expose, expose },
	{ FocusIn, focusIn },
	{ KeyPress, keyPress },
	{ MappingNotify, mappingNotify },
	{ MapRequest, mapRequest },
	{ MotionNotify, motionNotify },
	{ PropertyNotify, propertyNotify },
	{ UnmapNotify, unmapNotify }
};


void mwm::run() {
	XEvent event;

	XSync(display, false);
	while (running && !XNextEvent(display, &event)) {
		if (handler[event.type]) {
			std::cout << "EVENT: ";
			handler[event.type](&event);
		}
	}
}
