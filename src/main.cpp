#include "mwm.hpp"
#include "classes.hpp"

extern std::vector<Workspace*> workspaces;

bool running = true;
mwm wm;

int main() {
	wm.Workspaces = std::move(workspaces);
	wm.monitors = {
		Monitor()
	};
	wm.monitors.at(0).selectedWorkspace = wm.Workspaces.at(0);
	wm.monitors.at(0).selectedClient = NULL;

	wm.run();
	return 0;
}
