#include "widget.hpp"

widget::~widget()
{

}

notify_result widget::notify(SDL_Event const &)
{
	return failure;
}

void widget::render()
{
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &bounds);

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderDrawRect(renderer, &bounds);
}
