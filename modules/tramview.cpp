#include "tramview.hpp"

void tramview::init()
{
	add_back_button();

	background = IMG_LoadTexture(renderer, (resource_root / "tram" / "background.png").c_str());
	assert(background != nullptr);


}

void tramview::render()
{
	SDL_RenderCopy(renderer, background, nullptr, nullptr);

	gui_module::render();
}
