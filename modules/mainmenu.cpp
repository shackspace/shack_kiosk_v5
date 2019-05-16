#include "mainmenu.hpp"
#include "widgets/button.hpp"
#include "modules/lightroom.hpp"
#include "modules/tramview.hpp"
#include "modules/powerview.hpp"
#include "http_client.hpp"
#include "rendering.hpp"

#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>
#include <atomic>

namespace
{
	using nlohmann::json;

	int constexpr item_padding = 50;

	std::mutex keyholder_lock;

	bool is_open = false;
	std::string keyholder = "???";

	void update_keyholder(http_client & client)
	{

		auto data = client.transfer(
			client.get,
			"http://portal.shack:8088/status"
		);
		if(not data)
			return;
		try
		{
			// {"status":"open","keyholder":"xq","timestamp":1558039501604}
			auto cfg = json::parse(data->begin(), data->end());

			std::lock_guard _ { keyholder_lock };
			is_open = cfg["status"] == "open";
			keyholder = cfg["keyholder"];
		}
		catch(...)
		{

		}
	}

	void loop()
	{
		http_client client;
		client.set_headers({
			{ "Content-Type", "application/json" },
			{ "Access-Control-Allow-Origin", "*" },
		});

		while(true)
		{
			update_keyholder(client);

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

template<typename Target>
static button * set(button * b, SDL_Color color, char const * file)
{
	b->color = color;
	b->icon = IMG_LoadTexture(renderer, (resource_root / "icons" / file).c_str());
	b->on_click = []() {
		module::activate<Target>();
	};
	return b;
}

void mainmenu::init()
{
	SDL_Color palette[8] =
	{
	  { 0xE0, 0x40, 0xFB, 0xFF },
	  { 0xFF, 0x57, 0x22, 0xFF },
	  { 0x00, 0x96, 0x88, 0xFF },
	  { 0x8B, 0xC3, 0x4A, 0xFF },
	  { 0xFF, 0xC1, 0x07, 0xFF },
	  { 0xFF, 0x40, 0x81, 0xFF },
	  { 0x9C, 0x27, 0xB0, 0xFF },
	  { 0x03, 0xA9, 0xF4, 0xFF },
	};

	SDL_Color * col = palette;

	center_widgets.push_back(set<lightroom>(add<button>(), *col++, "lightbulb-on.png"));
	center_widgets.push_back(set<tramview>(add<button>(), *col++, "tram.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "bottle-wine.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "information.png"));

	center_widgets.push_back(set<powerview>(add<button>(), *col++, "flash.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "volume-high.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "cellphone-key.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "alert.png"));

	key_icon = IMG_LoadTexture(renderer, (resource_root / "icons" / "key-variant.png" ).c_str());

	std::thread(loop).detach();
}

void mainmenu::layout()
{
	auto const center_off = glm::ivec2(0, 100);
	auto const center_size = screen_size - 2 * center_off;

	SDL_Rect const top_bar = { 0, 0, screen_size.x, center_off.y };
	SDL_Rect const bottom_bar = { 0, screen_size.y - center_off.y - 1, screen_size.x, center_off.y };

	// layout the middle part
	{
		int const rows = (center_widgets.size() + 3) / 4;
		int const size = std::min(
			(center_size.x - item_padding) / 4,
			(center_size.y - item_padding) / rows
		);

		int const dx = (center_size.x - size * 4 - item_padding) / 2;
		int const dy = (center_size.y - size * rows - item_padding) / 2;

		for(size_t i = 0; i < center_widgets.size(); i++)
		{
			size_t x = i % 4;
			size_t y = i / 4;

			auto & bounds = center_widgets[i]->bounds;
			bounds.x = center_off.x + dx + size * x + item_padding;
			bounds.y = center_off.y + dy + size * y + item_padding;
			bounds.w = size - item_padding;
			bounds.h = size - item_padding;

		}
	}

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(renderer, &top_bar);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &bottom_bar);
}

void mainmenu::render()
{
	auto const center_off = glm::ivec2(0, 100);
	auto const center_size = screen_size - 2 * center_off;

	SDL_Rect const top_bar = { 0, 0, screen_size.x, center_off.y };
	SDL_Rect const bottom_bar = { 0, screen_size.y - center_off.y - 1, screen_size.x, center_off.y };

	gui_module::render();

	SDL_Rect bottom_modules[] =
	{
	  { 0 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  { 1 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  { 2 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	};

	// Module 0
	{
		SDL_Rect left = bottom_modules[0];
		left.w = left.h;

		left.x += 10;
		left.y += 10;
		left.w -= 20;
		left.h -= 20;

		SDL_Rect name = bottom_modules[0];
		name.x += bottom_modules[0].h;
		name.w -= bottom_modules[0].h;

		SDL_SetRenderDrawColor(renderer, 32, 32, 32, 0xFF);
		SDL_RenderFillRect(
			renderer,
			&bottom_modules[0]
		);

		SDL_RenderCopy(
			renderer,
			key_icon,
			nullptr,
			&left
		);
		{
			std::lock_guard _ { keyholder_lock };
			rendering::big_font->render(
				name,
				keyholder
			);
		}
	}
}
