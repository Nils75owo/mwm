#include "utils.hpp"

static int (*xerrorxlib)(Display *, XErrorEvent *);

void exitError(const char* err) {
	std::cerr << err << std::endl;
	exit(1);
}

int xerror(Display *dpy, XErrorEvent *ee) {
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	std::cerr << "dwm: fatal error: request code=" << ee->request_code << "error code=" << ee->error_code << std::endl;
	return xerrorxlib(dpy, ee); /* may call exit */
}


// ===============================
// check if an other wm is running
// ===============================
static int xerrorhandle(Display *dpy, XErrorEvent *ee) { 
	exitError("Other wm running");
	return -1;
}
void otherWMRunning(Display* display) {
	xerrorxlib = XSetErrorHandler(xerrorhandle);
	// this causes an error if some other window manager is running
	XSelectInput(display, DefaultRootWindow(display), SubstructureRedirectMask);
	XSync(display, False);
	XSetErrorHandler(xerror);
	XSync(display, False);
}

