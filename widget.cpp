#include "widget.hpp"

widget::~widget()
{

}

void widget::notify(SDL_Event const &)
{

}

void widget::render()
{
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &bounds);

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderDrawRect(renderer, &bounds);
}
