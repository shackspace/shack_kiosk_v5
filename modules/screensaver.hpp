#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

#include "module.hpp"

struct screensaver : module
{
	int effect;
	SDL_Texture * logo;
	double timer;

	void next_effect();

	void init() override;

	void enter() override;

	void render() override;

	notify_result notify(SDL_Event const & ev) override;
};

#endif // SCREENSAVER_HPP
