#ifndef LIGHTROOM_HPP
#define LIGHTROOM_HPP

#include "gui_module.hpp"
#include <array>

struct lightroom : gui_module
{
	struct switch_t {
		uint8_t bitnum;
		std::vector<SDL_Rect> rects;
		bool is_on = false;
		double power = 0.0;
	};

	SDL_Texture* background;
	std::array<SDL_Texture*, 4> foregrounds;
	SDL_Texture * switch_background;
	std::array<SDL_Texture*, 8> switches;

	std::array<switch_t, 8> switch_config;

	void init() override;

	void notify(SDL_Event const & ev) override;

	void render() override;
};

#endif // LIGHTROOM_HPP
