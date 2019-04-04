#ifndef GUI_MODULE_HPP
#define GUI_MODULE_HPP

#include "module.hpp"
#include "widget.hpp"

#include <vector>
#include <memory>

struct gui_module : module
{
	std::vector<std::unique_ptr<widget>> widgets;

	void add_back_button();

	notify_result notify(SDL_Event const & ev) override;

	//! should lay out the module. called whenever screen size changes
	virtual void layout();

	void render() override;

	//! adds a widget of type `T`.
	template<typename T>
	T * add()
	{
		return static_cast<T*>(widgets.emplace_back(std::make_unique<T>()).get());
	}
};

#endif // GUI_MODULE_HPP
