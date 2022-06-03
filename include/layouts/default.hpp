#pragma once
#include "classes.hpp"

#define rootC static_cast<Block*>(base->root)

namespace layouts {


struct test : Layout {
	void init(Workspace *base) {
		base->root = new Block();

		rootC->direction = 'x';
		rootC->x = 0;
		rootC->y = 0;
		rootC->w = base->monitor->monW;
		rootC->h = base->monitor->monH;
	}

	void addBlock(Workspace *base, _BaseBlock *block) {
		rootC->content.push_back(block);
		updateLayout(base);
	}

	void removeWindow(Workspace *base) {}

	void updateLayout(Workspace *base) {
		for (int i = 0; i < rootC->content.size(); ++i) {
			// Equally distribute the size
			rootC->content.at(i)->size = 100.f / rootC->content.size();

			rootC->content.at(i)->x = ((float)base->monitor->winW / 100) * (100.f / rootC->content.size()) * i;
			rootC->content.at(i)->y = base->monitor->winY;

			rootC->content.at(i)->w = ((float)base->monitor->winW / 100) * (100.f / rootC->content.size());
			rootC->content.at(i)->h = base->monitor->winH;
		}
	}
};


}
