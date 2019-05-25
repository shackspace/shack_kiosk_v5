#ifndef INFOVIEW_HPP
#define INFOVIEW_HPP

#include "gui_module.hpp"

//!
//! Displays the following information:
//! - who is there (shackles)
//! - when is the next trash collection (gelber sack, restmüll)
//! - what is the radiation levels
//! - what is the feinstaub levels
//!
struct infoview : gui_module
{
	void init() override;

	void render() override;
};

#endif // INFOVIEW_HPP
