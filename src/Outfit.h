#pragma once

#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;

class Outfit
{
public:
	Outfit();
	Outfit(ColorA8u shirt, ColorA8u pants);
	ColorA8u shirtColor;
	ColorA8u pantColor;
	int appearances;

	void update(ColorA8u shirt, ColorA8u pants);
};

