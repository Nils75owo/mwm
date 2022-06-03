#pragma once
#include "include.hpp"

typedef class Workspace Workspace;
typedef class Client Client;
typedef class Block Block;
typedef class _BaseBlock _BaseBlock;

extern const int minWindowX;
extern const int minWindowY;
extern Display* display;

struct Monitor {
	int monX, monY, monW, monH;
	int winX, winY, winW, winH;

	Workspace* selectedWorkspace;
	Client* selectedClient;

	Monitor() {
		monX = monY = winX = winY = 0;
		monW = winW = 1920;
		monH = winH = 1080;
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
};

struct Layout {
	void init(Workspace *base);
	void addBlock(Workspace *base, _BaseBlock *block);
	void removeWindow(Workspace *base);
	void updateLayout(Workspace *base);
};
