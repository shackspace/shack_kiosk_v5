#include "lightroom.hpp"
#include "mainmenu.hpp"
#include "widgets/button.hpp"

#include <algorithm>

void lightroom::init()
{
	backgrounds =
	{
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0000.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0001.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0010.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0011.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0100.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0101.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0110.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0111.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1000.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1001.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1010.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1011.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1100.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1101.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1110.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1111.png" ).c_str()),
	};

	switch_background = IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switches.png" ).c_str());

	switches =
	{
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch0.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch1.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch2.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch3.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch4.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch5.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch6.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "switch7.png" ).c_str()),
	};
	for(auto tex : switches)
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);

	switch_config =
	{
	  switch_t { 0x01, { SDL_Rect { 72, 686, 418, 280 } } }, // unten links
	  switch_t { 0x02, { SDL_Rect { 558, 516, 343, 210 } } }, // unten mitte
	  switch_t { 0x02, { SDL_Rect { 943, 380, 302, 174 } } }, // unten rechts
	  switch_t { 0x02, { SDL_Rect { 804, 304, 112, 87 }, SDL_Rect { 290, 420, 324, 171 } } }, // mitte
	  switch_t { 0x08, { SDL_Rect { 650, 159, 248, 119 } } }, // hinten rechts
	  switch_t { 0x08, { SDL_Rect { 552, 73, 232, 101 } } }, // ganz hinten rechts
	  switch_t { 0x04, { SDL_Rect { 247, 152, 259, 114 } } }, // ganz hinten links
	  switch_t { 0x04, { SDL_Rect { 325, 252, 281, 138 } } }, // hinten links
	};

	auto * btn = add<button>();
	btn->bounds = { 10, 10, 200, 200 };
	btn->icon = home_icon;
	btn->color = { 0x03, 0xA9, 0xF4, 255 };
	btn->on_click = []() {
		change<mainmenu>();
	};
}

void lightroom::notify(SDL_Event const & ev)
{
	SDL_Rect area = { 0, 0, 1280, 1024 };
	area.x = (screen_size.x - area.w) / 2;
	area.y = (screen_size.y - area.h) / 2;

	if(ev.type == SDL_MOUSEBUTTONDOWN)
	{
		SDL_Point pt { ev.button.x - area.x, ev.button.y - area.y };

		for(auto & sw : switch_config)
		{
			bool toggle = false;
			for(auto const & rect : sw.rects)
				toggle |= SDL_PointInRect(&pt, &rect);
			sw.is_on ^= toggle;
		}

		// light_config = (light_config + 1) % 16;
	}

	gui_module::notify(ev);
}

void lightroom::render()
{
	for(auto & sw : switch_config)
	{
		sw.power = std::clamp(sw.power + 2.0 * (sw.is_on ? 1 : -1) * time_step, 0.0, 1.0);
	}

	uint8_t light_config = 0;
	for(auto const & sw : switch_config)
		light_config |= (sw.is_on ? sw.mask : 0);

	SDL_Rect area = { 0, 0, 1280, 1024 };
	area.x = (screen_size.x - area.w) / 2;
	area.y = (screen_size.y - area.h) / 2;

	SDL_RenderCopy(renderer, backgrounds.at(light_config), nullptr, &area);
	SDL_RenderCopy(renderer, switch_background, nullptr, &area);

	int a;
	for(size_t i = 0; i < 8; i++)
	{
		auto power = switch_config[i].power;

		a = std::clamp<int>(2.0 * 255 * power, 0, 255);

		SDL_SetTextureAlphaMod(switches[i], a);
		SDL_SetTextureColorMod(switches[i], 0x00, 0xFF, 0x00);
		SDL_RenderCopy(renderer, switches[i], nullptr, &area);

		a = std::clamp<int>(255 * std::clamp(1.0 - std::pow(power, switch_config[i].is_on ? 0.5 : 2.0), 0.0, 1.0), 0, 255);

		SDL_SetTextureAlphaMod(switches[i], a);
		SDL_SetTextureColorMod(switches[i], 0xFF, 0x00, 0x00);
		SDL_RenderCopy(renderer, switches[i], nullptr, &area);

	}

	gui_module::render();
}

