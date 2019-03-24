#ifndef WIDGET_HPP
#define WIDGET_HPP

#include "kiosk.hpp"

struct widget
{
	SDL_Rect bounds;

	virtual ~widget();

	virtual void notify(SDL_Event const & ev);

	virtual void render();
};

#endif // WIDGET_HPP
