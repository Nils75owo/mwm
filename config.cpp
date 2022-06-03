#include "config.hpp"
#include "layouts/default.hpp"

const int minWindowX = 10;
const int minWindowY = 10;

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
