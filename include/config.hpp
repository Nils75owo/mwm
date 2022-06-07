#pragma once
#include "classes.hpp"
#include "mwm.hpp"

extern const int minWindowX;
extern const int minWindowY;
extern const int borderWidth;
extern std::map<const char*, const char*> colors;

extern Layout defaultLayout;

extern std::vector<Key> keys;
extern std::vector<Button> buttons;

void starup();
