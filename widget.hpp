#ifndef WIDGET_HPP
#define WIDGET_HPP

#include "kiosk.hpp"
#include "module.hpp"

struct widget
{
	SDL_Rect bounds;

	virtual ~widget();

	virtual notify_result notify(SDL_Event const & ev);

	virtual void render();
};

#endif // WIDGET_HPP
