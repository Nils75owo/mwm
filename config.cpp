#include "config.hpp"
#include "layouts/default.hpp"

#define MODKEY Mod4Mask

const int minWindowX = 10;
const int minWindowY = 10;
const int borderWidth = 2;

Layout defaultLayout = layouts::test();

std::vector<Workspace*> workspaces = {
	new Workspace("1"),
	new Workspace("2"),
	new Workspace("3"),
	new Workspace("4"),
	new Workspace("5"),
	new Workspace("6"),
	new Workspace("7"),
	new Workspace("8"),
	new Workspace("9"),
	new Workspace("10"),
};

std::map<const char*, const char*> colors {
	{ "border", "#222222" }
};

void spawn(std::vector<const char*> arg) {
	for (const char* c : arg) std::cout << c << std::endl;

	if (fork() == 0) {
		if (wm.display)
			close(ConnectionNumber(wm.display));
		setsid();
		execvp(arg[0], (char **)&arg[0]);
		fprintf(stderr, "dwm: execvp %s", ((char **)&arg)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

std::vector<Key> keys = {
	{ MODKEY, XK_i, spawn, {"kitty"} }
};
std::vector<Button> buttons = {
};

void startup() {
	const char* picom[] = {"picom"};
	spawn({"picom"});
	spawn({"bgl"});
}
