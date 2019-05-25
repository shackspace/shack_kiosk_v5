#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "gui_module.hpp"

struct button;

//!
//! Central menu of the kiosk.
//!
//! Allows going into submodules, also
//! views some information about:
//! - power use
//! - who is keyholder
//! - what music is currently playing
//!
struct mainmenu : gui_module
{
	std::vector<widget*> center_widgets;

	SDL_Texture * key_icon;
	SDL_Texture * power_icon;
	SDL_Texture * skull_icon;

	SDL_Texture * volumio_icon_song;
	SDL_Texture * volumio_icon_artist;
	SDL_Texture * volumio_icon_album;
	SDL_Texture * volumio_play;
	SDL_Texture * volumio_pause;
	SDL_Texture * volumio_next;

	// button * songbutton;
	button * playpausebutton;

	SDL_Texture * volumio_albumart_none = nullptr;
	SDL_Texture * volumio_albumart = nullptr;

	int module_cycle = 0;
	bool modules_cycling = 0;
	float module_cycle_progress = 0.0f;

	void init() override;

	void layout() override;

	void render() override;

	void render_power_module(SDL_Rect module_rect);
	void render_keyholder_module(SDL_Rect module_rect);
	void render_trash_module(SDL_Rect module_rect);
	void render_event_module(SDL_Rect module_rect);
};

#endif // MAINMENU_HPP
