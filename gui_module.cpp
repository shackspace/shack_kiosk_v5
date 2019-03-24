#include "gui_module.hpp"



void gui_module::notify(SDL_Event const & ev)
{
	SDL_Point pt;
	switch(ev.type)
	{
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			pt = { ev.button.x, ev.button.y };
			break;

		case SDL_MOUSEMOTION:
			pt = { ev.motion.x, ev.motion.y };
			break;

		case SDL_MOUSEWHEEL:
			pt = { ev.wheel.x, ev.wheel.y };
			break;

		default: // no UI-relevant event:
			return;
	}

	for(auto it = widgets.rbegin(); it != widgets.rend(); it++)
	{
		auto * ptr = it->get();

		if(not SDL_PointInRect(&pt, &ptr->bounds))
			continue;

		(*it)->notify(ev);
		break;
	}
}

void gui_module::render()
{
	layout();

	for(auto const & w : widgets)
		w->render();
}


void gui_module::layout()
{

}
