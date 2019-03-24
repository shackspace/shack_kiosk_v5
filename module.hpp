#ifndef MODULE_HPP
#define MODULE_HPP

#include "kiosk.hpp"
#include <SDL.h>
#include <optional>

struct module
{
	virtual ~module();

	//! called when the module is first used
	virtual void init();

	//! called when the module is about to show
	virtual void enter();

	//! called when an SDL event happens.
	virtual void notify(SDL_Event const & ev);

	//! should draw the module
	virtual void render();

	//! called when the module is not shown anymore
	virtual void leave();

private:
	static void change(module * other);
public:
	//! Switches to the given module.
	template<typename T>
	static void change()
	{
		static std::optional<T> module;
		if(not module)
		{
			module.emplace();
			module->init();
		}
		change(&module.value());
	}
};

#endif // MODULE_HPP
