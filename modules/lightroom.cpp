#include "lightroom.hpp"
#include "mainmenu.hpp"
#include "widgets/button.hpp"

#include "http_client.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <nlohmann/json.hpp>

static std::mutex commands_mutex;
static std::queue<std::tuple<int,bool>> commands;

static void query_thread()
{
	using nlohmann::json;
	http_client client;

	client.set_headers({
		{ "Content-Type", "application/json" },
		{ "Access-Control-Allow-Origin", "*" },
	});

	while(true)
	{
		auto const lroom = module::get<lightroom>();

		//*/
		for(auto & sw : lroom->switch_config)
		{
			auto data = client.transfer(
				client.get,
				"http://openhab.shack/lounge/" + std::to_string(sw.group_index)
			);
			if(not data)
				continue;
			auto cfg = json::parse(data->begin(), data->end());

			sw.is_on = (cfg["state"] == "on");
		}
		//*/

		for(size_t i = 0; i < 100; i++)
		{
			if(std::lock_guard _ { commands_mutex }; not commands.empty())
			{
				auto cmd = commands.front();
				json payload = { {  "state", std::get<1>(cmd) ? "on" : "off" } };

				auto string = payload.dump();

				auto success = client.transfer(
					client.put,
					"http://openhab.shack/lounge/" + std::to_string(std::get<0>(cmd)),
					ro_buffer<const std::byte> { reinterpret_cast<std::byte const *>(string.data()), string.size() }
				);
				if(not success)
					fprintf(stderr, "i failed hard.\n");
				else
					fprintf(stderr, "%.*s\n", success->size(), success->data());
				fflush(stderr);

				commands.pop();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void lightroom::init()
{
	add_back_button();

	background = IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0000.png" ).c_str());
	foregrounds =
	{
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0001.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0010.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone0100.png" ).c_str()),
	  IMG_LoadTexture(renderer, (resource_root / "lightroom" / "zone1000.png" ).c_str()),
	};

	auto const blendmode = SDL_ComposeCustomBlendMode(
		SDL_BLENDFACTOR_SRC_ALPHA,
		SDL_BLENDFACTOR_ONE,
		SDL_BLENDOPERATION_MAXIMUM,
		SDL_BLENDFACTOR_SRC_ALPHA,
		SDL_BLENDFACTOR_ONE,
		SDL_BLENDOPERATION_MAXIMUM
	);
	for(auto tex : foregrounds)
	{
		//if(SDL_SetTextureBlendMode(tex, blendmode) < 0)
		//	die("%s", SDL_GetError());
		SDL_SetTextureAlphaMod(tex, 255);
	}

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
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	switch_config =
	{
	  switch_t { 0, 8, { SDL_Rect { 72, 686, 418, 280 } } }, // unten links
	  switch_t { 1, 7, { SDL_Rect { 558, 516, 343, 210 } } }, // unten mitte
	  switch_t { 1, 6, { SDL_Rect { 943, 380, 302, 174 } } }, // unten rechts
	  switch_t { 1, 5, { SDL_Rect { 804, 304, 112, 87 }, SDL_Rect { 290, 420, 324, 171 } } }, // mitte
	  switch_t { 3, 3, { SDL_Rect { 650, 159, 248, 119 } } }, // hinten rechts
	  switch_t { 3, 1, { SDL_Rect { 552, 73, 232, 101 } } }, // ganz hinten rechts
	  switch_t { 2, 2, { SDL_Rect { 247, 152, 259, 114 } } }, // ganz hinten links
	  switch_t { 2, 4, { SDL_Rect { 325, 252, 281, 138 } } }, // hinten links
	};
	std::thread(query_thread).detach();
}

notify_result lightroom::notify(SDL_Event const & ev)
{
	SDL_Rect area = { 0, 0, 1280, 1024 };
	area.x = (screen_size.x - area.w) / 2;
	area.y = (screen_size.y - area.h) / 2;

	if(ev.type == SDL_MOUSEBUTTONDOWN)
	{
		SDL_Point pt { ev.button.x - area.x, ev.button.y - area.y };

		bool any = false;
		for(size_t i = 0; i < switch_config.size(); i++)
		{
			auto & sw = switch_config.at(i);
			bool toggle = false;
			for(auto const & rect : sw.rects)
				toggle |= SDL_PointInRect(&pt, &rect);
			sw.is_on ^= toggle;
			any |= toggle;

			if(toggle)
			{
				std::lock_guard _ { commands_mutex };
				commands.emplace(sw.group_index, sw.is_on);
			}
		}

		if(any)
			return success;

		// light_config = (light_config + 1) % 16;
	}

	return gui_module::notify(ev);
}

void lightroom::render()
{
	std::array<double, 4> blendweights = { 0, 0, 0, 0 };

	for(auto & sw : switch_config)
	{
		sw.power = std::clamp(sw.power + 4.0 * (sw.is_on ? 1 : -1) * time_step, 0.0, 1.0);

		blendweights[sw.bitnum] = std::max(
			blendweights[sw.bitnum],
			sw.power
		);
	}

	SDL_Rect area = { 0, 0, 1280, 1024 };
	area.x = (screen_size.x - area.w) / 2;
	area.y = (screen_size.y - area.h) / 2;

	// Fill background with "default pattern"
	SDL_RenderCopy(renderer, background, nullptr, &area);
	for(size_t i = 0; i < blendweights.size(); i++)
	{
		auto tex = foregrounds.at(i);
		auto const gray = uint8_t(std::clamp(255.0 * blendweights[i], 0.0, 255.0));
		SDL_SetTextureColorMod(tex, gray, gray, gray);
		SDL_RenderCopy(renderer, tex, nullptr, &area);
	}

	// override the background image with 100% black switches
	SDL_RenderCopy(renderer, switch_background, nullptr, &area);

	for(size_t i = 0; i < 8; i++)
	{
		auto const power = switch_config[i].power;
		auto const fcol = glm::mix(
			glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0),
			power
		);
		auto const map = [](float f) { return Uint8(std::clamp(255.0f * f, 0.0f, 255.0f)); };

		SDL_SetTextureColorMod(switches[i], map(fcol.r), map(fcol.g), map(fcol.b));
		SDL_RenderCopy(renderer, switches[i], nullptr, &area);
	}

	gui_module::render();
}

