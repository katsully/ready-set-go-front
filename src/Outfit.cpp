#include "Outfit.h"

Outfit::Outfit()
{
}

Outfit::Outfit(ColorA8u shirt, ColorA8u pants) 
{
	shirtColor = shirt;
	pantColor = pants;
	active = true;
	activeCount = 2000;
}

void Outfit::reset()
{
	active = true;
	activeCount = 2000;
}

void Outfit::update(ColorA8u shirt, ColorA8u pants)
{
	shirtColor = shirt;
	pantColor = pants;
	active = true;
	activeCount = 2000;
}
