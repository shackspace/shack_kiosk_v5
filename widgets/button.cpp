#include "button.hpp"

int constexpr icon_padding = 25;
int constexpr border_width = 3;

notify_result button::notify(SDL_Event const & ev)
{
	if(ev.type == SDL_MOUSEBUTTONDOWN)
	{
		if(on_click)
			on_click();
		return success;
	}
	return widget::notify(ev);
}

void button::render()
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &bounds);

	SDL_Rect border = bounds;
	for(int i = 0; i < border_width; i++)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &border);
		border.x += 1;
		border.y += 1;
		border.w -= 2;
		border.h -= 2;
	}

	if(background != nullptr)
	{
		SDL_RenderCopy(renderer, background, nullptr, &border);
	}

	if(icon != nullptr)
	{
		auto area = bounds;
		area.x += icon_padding;
		area.y += icon_padding;
		area.w -= 2 * icon_padding;
		area.h -= 2 * icon_padding;

		SDL_SetTextureColorMod(icon, icon_tint.r, icon_tint.g, icon_tint.b);
		SDL_SetTextureAlphaMod(icon, icon_tint.a);
		SDL_RenderCopy(renderer, icon, nullptr, &area);
	}
}
