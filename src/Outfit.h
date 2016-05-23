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
	bool active;
	int activeCount;

	void reset();
	void update(ColorA8u shirt, ColorA8u pants);
};

