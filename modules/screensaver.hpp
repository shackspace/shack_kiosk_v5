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

	void notify(SDL_Event const & ev);
};

#endif // SCREENSAVER_HPP
