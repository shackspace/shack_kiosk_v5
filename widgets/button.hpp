#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "widget.hpp"

#include <functional>

struct button : widget
{
	std::function<void()> on_click;

	SDL_Texture * icon = nullptr;
	SDL_Color color = { 0x00, 0xBE, 0x00, 0xFF };

	void notify(SDL_Event const & ev) override;

	void render() override;
};

#endif // BUTTON_HPP
