#include "mwm.hpp"

extern std::vector<Workspace*> workspaces;
Display* display;

bool running = true;

int main() {
	mwm wm;
	wm.Workspaces = std::move(workspaces);
	wm.monitors = {
		Monitor()
	};
	wm.monitors.at(0).selectedWorkspace = wm.Workspaces.at(0);
	wm.monitors.at(0).selectedClient = NULL;

	wm.run();
	return 0;
}
