#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "gui_module.hpp"

struct button;

struct mainmenu : gui_module
{
	std::vector<widget*> center_widgets;

	SDL_Texture * key_icon;

	SDL_Texture * volumio_icon_song;
	SDL_Texture * volumio_icon_artist;
	SDL_Texture * volumio_icon_album;
	SDL_Texture * volumio_play;
	SDL_Texture * volumio_pause;
	SDL_Texture * volumio_next;

	button * songbutton;

	SDL_Texture * volumio_albumart_none = nullptr;
	SDL_Texture * volumio_albumart = nullptr;

	void init() override;

	void layout() override;

	void render() override;
};

#endif // MAINMENU_HPP
