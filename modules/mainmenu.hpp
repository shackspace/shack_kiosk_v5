#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "gui_module.hpp"

struct mainmenu : gui_module
{
	std::vector<widget*> center_widgets;

	SDL_Texture * key_icon;

	void init() override;

	void layout() override;

	void render() override;
};

#endif // MAINMENU_HPP
