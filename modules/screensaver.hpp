#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

#include "module.hpp"

//!
//! Is fancy and wants attention.
//!
//! This module will be activated after about
//! a minute of inactivity.
//!
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
