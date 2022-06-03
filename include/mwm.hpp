#pragma once

#include "include.hpp"
#include "classes.hpp"
#include "utils.hpp"

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

struct mwm {
	Display *display;
	Window root;
	Window wmcheckwin;
	int screen;

	int displayWidth;
	int displayHeight;

	int tmpWorkspaces;

	std::vector<Monitor> monitors;

	Atom wmatom[WMLast], netatom[NetLast];
	Cur *cursor[CurLast];

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
