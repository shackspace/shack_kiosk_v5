#ifndef POWERVIEW_HPP
#define POWERVIEW_HPP

#include "gui_module.hpp"
#include <array>

struct powerview : gui_module
{
	double total_power = 0.0;

	void init() override;

	void render() override;
};

#endif // POWERVIEW_HPP
