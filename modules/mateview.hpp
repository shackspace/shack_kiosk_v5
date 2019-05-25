#ifndef MATEVIEW_HPP
#define MATEVIEW_HPP

#include "gui_module.hpp"

//!
//! Displays the remaining bottles
//! in the matomat as a chart.
//!
struct mateview : gui_module
{
	void init() override;

	void render() override;
};

#endif // MATEVIEW_HPP
