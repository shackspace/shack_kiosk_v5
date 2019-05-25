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
	struct MuellInfo
	{
		tm papiermuell, restmuell, gelber_sack;
	};

	void init() override;

	void render() override;

	MuellInfo get_muell_info() const;
};

#endif // INFOVIEW_HPP
