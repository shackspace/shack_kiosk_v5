#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "gui_module.hpp"

struct mainmenu : gui_module
{
	std::vector<widget*> center_widgets;

	void init() override;

	void layout() override;
};

#endif // MAINMENU_HPP
