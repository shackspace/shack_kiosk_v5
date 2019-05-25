#ifndef TRAMVIEW_HPP
#define TRAMVIEW_HPP

#include "gui_module.hpp"

//!
//! Shows the next four connections of
//! tram and night bus routes.
//!
struct tramview : gui_module
{
	SDL_Texture * background;

	std::array<SDL_Texture*, 8> route_icons;

	void init() override;

	void render() override;
};

#endif // TRAMVIEW_HPP
