#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "widget.hpp"

#include <functional>

struct button : widget
{
	std::function<void()> on_click;

	SDL_Texture * icon = nullptr;
	SDL_Texture * background = nullptr;
	SDL_Color icon_tint = { 0x00, 0x00, 0x00, 0xFF };
	SDL_Color color = { 0x00, 0xBE, 0x00, 0xFF };

	notify_result notify(SDL_Event const & ev) override;

	void render() override;
};

#endif // BUTTON_HPP
