#ifndef POWERVIEW_HPP
#define POWERVIEW_HPP

#include "gui_module.hpp"
#include <array>

//!
//! Displays the current power usage
//! of the shackspace in a line diagram.
//!
//! Separate lines for L1, L2, L3 and summed up power.
//!
struct powerview : gui_module
{
	double total_power = 0.0;

	void init() override;

	void render() override;
};

#endif // POWERVIEW_HPP
