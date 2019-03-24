#ifndef MODULE_HPP
#define MODULE_HPP

#include "kiosk.hpp"
#include <SDL.h>
#include <optional>

struct module
{
public:
	virtual ~module();

	virtual void init();

	virtual void enter();

	virtual void notify(SDL_Event const & ev);

	virtual void render();

	virtual void leave();

private:
	static void change(module * other);
public:
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
