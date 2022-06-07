#ifndef CLASSES_HPP_
#define CLASSES_HPP_

#include "include.hpp"
#include "mwm.hpp"

typedef class Workspace Workspace;
typedef class Client Client;
typedef class Block Block;
typedef class _BaseBlock _BaseBlock;

extern const int minWindowX;
extern const int minWindowY;

struct Monitor {
	int monX, monY, monW, monH;
	int winX, winY, winW, winH;

	Workspace* selectedWorkspace;
	Client* selectedClient;

	Monitor() {
		monX = monY = winX = winY = 0;
		monW = winW = 1024;
		monH = winH = 600;
		//monW = winW = 1920;
		//monH = winH = 1080;
	}
};

struct Workspace {
	const char* name;

	_BaseBlock* root;
	Client* selectedClient;

	std::function<void(Workspace&)> tile;

	Monitor* monitor;

	Workspace(const char* name) {
		name = name;
	}
};

struct _BaseBlock {
	// HACK: store the type
	char type = '#';
	int x, y, w, h;
	double size = 100.f;
	Block* master;
	bool isFixed, isCentered, isFloating, isUrgent, isFullscreen;

	Monitor* monitor;
};

struct Block : _BaseBlock {
	char type = 'b';
	char direction = 'x';
	int x, y, w, h;

	std::vector<_BaseBlock*> content;
};

struct Client : _BaseBlock {
	char type = 'w';
	char name[256];
	Window win;

	void setState(long state) {
		long data[] = { state, None };
		XChangeProperty(
			wm.display,
			win,
			wm.wmatom[WMState],
			wm.wmatom[WMState],
			32,
			PropModeReplace,
			(unsigned char*)data,
			2
		);
	}
};

struct Layout {
	void init(Workspace *base);
	void addBlock(Workspace *base, _BaseBlock *block);
	void removeWindow(Workspace *base);
	void updateLayout(Workspace *base);
};

#endif
