#include "mwm.hpp"
#define callHook(hook) if (hook != nullptr) hook(*this)

typedef void (*handlerFunction)(XEvent* event);

Cur::Cur(int shape) {
	cursor = XCreateFontCursor(display, shape);
}

mwm::mwm() {
	callHook(hookPreeInit);

	Atom utf8string;
	XSetWindowAttributes windowAttributes;

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

	callHook(hookPostInit);
}

mwm::~mwm() {}

void buttonPress(XEvent* event) {
	switch (event->xbutton.button) {
		case XK_q:
			running = false;
			break;
	}
}
void clientMessage(XEvent* event) {}
void configureRequest(XEvent* event) {}
void configureNotify(XEvent* event) {}
void destroyNotify(XEvent* event) {}
void enterNotify(XEvent* event) {}
void expose(XEvent* event) {}
void focusIn(XEvent* event) {}
void keyPress(XEvent* event) {}
void mappingNotify(XEvent* event) {}
void mapRequest(XEvent* event) {}
void motionNotify(XEvent* event) {}
void propertyNotify(XEvent* event) {}
void unmapNotify(XEvent* event) {}

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
			handler[event.type](&event);
		}
	}
}
