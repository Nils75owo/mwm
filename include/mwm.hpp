#ifndef MWM_HPP_
#define MWM_HPP_

#include "include.hpp"
#include "utils.hpp"
class Client;
class Monitor;
class Workspace;

extern bool running;

enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

struct Cur {
	Cursor cursor;

	Cur(int shape);
};

typedef struct mwm mwm;
extern mwm wm;

typedef struct {
	unsigned int mod;
	KeySym keysym;

	void (*func)(std::vector<const char *>);
	std::vector<const char*> args;
} Key;
typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;

	void (*func)(std::vector<const char *>);
	std::vector<const char*> args;
} Button;

struct mwm {
	Display *display;
	Window root;
	Window wmcheckwin;
	int screen;

	int displayWidth;
	int displayHeight;

	std::map<const char*, XftColor> xColors;

	unsigned int tmpWorkspaces;
	unsigned int numlockmask;

	std::vector<Monitor> monitors;

	Atom wmatom[WMLast], netatom[NetLast];
	Cur *cursor[CurLast];

	void initColors(std::map<const char*, const char*> &colors);

	void scan();
	void grabKeys();
	void grabButtons(Client *client, bool focused);
	void updateNumlockMaks();
	void configure(Client *client);
	void manage(Window win, XWindowAttributes *windoAttributes);
	void unmanage(Client *client, bool destroyed);

	Drawable drawable;
	GC gc;
public:
	std::vector<Workspace*> Workspaces;

	// Event hooks
	std::function<void(mwm&)> hookPreeInit = nullptr;
	std::function<void(mwm&)> hookPostInit = nullptr;
	std::function<void(mwm&)> hookPreeWindowCreate = nullptr;
	std::function<void(mwm&)> hookPostWindowCreate = nullptr;
	std::function<void(mwm&)> hookPreeDestroy = nullptr;
	std::function<void(mwm&)> hookPreeTile = nullptr;
	std::function<void(mwm&)> hookPostTile = nullptr;

	void run();

	mwm();
	~mwm();
};

#endif
