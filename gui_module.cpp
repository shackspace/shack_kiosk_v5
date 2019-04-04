#include "gui_module.hpp"
#include "widgets/button.hpp"
#include "modules/mainmenu.hpp"

notify_result gui_module::notify(SDL_Event const & ev)
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
			return module::notify(ev);
	}

	for(auto it = widgets.rbegin(); it != widgets.rend(); it++)
	{
		auto * ptr = it->get();

		if(not SDL_PointInRect(&pt, &ptr->bounds))
			continue;

		return (*it)->notify(ev);
	}

	return module::notify(ev);
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


void gui_module::add_back_button()
{
	auto * btn = add<button>();
	btn->bounds = { 10, 10, 200, 200 };
	btn->icon = home_icon;
	btn->color = { 0x03, 0xA9, 0xF4, 255 };
	btn->on_click = []() {
		activate<mainmenu>();
	};
}
