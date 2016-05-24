#include "Outfit.h"

Outfit::Outfit()
{
}

Outfit::Outfit(ColorA8u shirt, ColorA8u pants) 
{
	shirtColor = shirt;
	pantColor = pants;
	appearances = 1;
}

void Outfit::update(ColorA8u shirt, ColorA8u pants)
{
	shirtColor = shirt;
	pantColor = pants;
	appearances += 1;
}
