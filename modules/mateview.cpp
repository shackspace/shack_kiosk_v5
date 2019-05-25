#include "mateview.hpp"
#include "http_client.hpp"
#include "rendering.hpp"

#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>
#include <atomic>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace
{
	using std::chrono::system_clock;

	struct Shaft
	{
		std::string const title;
		int const api_index;
		SDL_Color color;
		std::atomic_bool fill_level_available = false;
		std::atomic_int fill_level = 0;
	};

	std::array shafts =
	{
	  Shaft { "Wasser",          1,  { 157, 192, 249, 0xFF } },
	  Shaft { "Mate Cola",       2,  { 114, 27, 12, 0xFF } },
	  Shaft { "Apfelschorle",    3,  { 0xf6, 0xc9, 0x28, 0xFF } },
	  Shaft { "Zitronensprudel", 4,  { 0xcb, 0xfa, 0xf7, 0xFF } },
	  Shaft { "Mate 1",          26, { 0xfa, 0xf3, 0x5c, 0xFF } },
	  Shaft { "Mate 2",          27, { 0xfa, 0xf3, 0x5c, 0xFF } },
	};

	void fetch(http_client & client, Shaft & shaft)
	{
		auto raw = client.transfer(
			client.get,
			"https://ora5.tutschonwieder.net/ords/lick_prod/v1/get/fuellstand/1/" + std::to_string(shaft.api_index)
		);
		if(not raw)
		{
			shaft.fill_level_available = false;
			return;
		}
		try
		{
			auto const json = nlohmann::json::parse(raw->begin(), raw->end());
			shaft.fill_level = json["fuellstand"];
			shaft.fill_level_available = true;
		}
		catch(...)
		{
			shaft.fill_level_available = false;
		}
	}

	[[noreturn]] void task()
	{
		http_client client;
		client.set_headers({
			{ "Content-Type", "application/json" },
			{ "Access-Control-Allow-Origin", "*" },
		});

		while(true)
		{
			try
			{
				for(size_t i = 0; i < shafts.size(); i++)
				{
					fetch(client, shafts[i]);
				}
			}
			catch (...)
			{
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}
}

void mateview::init()
{
	add_back_button();
	std::thread(task).detach();
}

void mateview::render()
{
	gui_module::render();


	SDL_Rect const window = { 220, 20, 1040, 840 };

	SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	SDL_RenderFillRect(renderer, &window);

	auto const get_column_rect = [&](size_t idx) -> SDL_Rect
	{
		SDL_Rect full_column = window;
		full_column.w /= shafts.size();
		full_column.x += full_column.w * int(idx);

		assert(full_column.x >= 50);

		full_column.x += 20;
		full_column.w -= 40;

		return full_column;
	};

	// Draw diagram
	{
		SDL_RenderSetClipRect(renderer, &window);

		int const max_fill_level = 200;
		auto const & font = *rendering::medium_font;
		for(size_t i = 0; i < shafts.size(); i++)
		{
			auto const & shaft = shafts[i];
			if(not shaft.fill_level_available)
				continue;

			auto full_column = get_column_rect(i);
			full_column.h = (full_column.h * shaft.fill_level) / max_fill_level;
			full_column.y = window.y + window.h - full_column.h;

			SDL_SetRenderDrawColor(renderer, shaft.color.r, shaft.color.g, shaft.color.b, shaft.color.a);
			SDL_RenderFillRect(renderer, &full_column);

			SDL_SetRenderDrawColor(renderer, shaft.color.r/2, shaft.color.g/2, shaft.color.b/2, shaft.color.a);
			SDL_RenderDrawRect(renderer, &full_column);

			SDL_Rect column_label;
			int alignment;
			if(full_column.h >= (window.h - font.height() - 10))
			{
				// "below the column top"
				column_label = full_column;
				column_label.y += 5;
				alignment = FontRenderer::Top | FontRenderer::Center;
			}
			else
			{
				// above the column
				column_label = full_column;
				column_label.y -= 2 * font.height();
				column_label.h = 2 * font.height();
				alignment = FontRenderer::Bottom | FontRenderer::Center;
			}

			font.render(
				column_label,
				std::to_string(shafts[i].fill_level),
				alignment
			);
		}
	}

	SDL_RenderSetClipRect(renderer, nullptr);

	// Draw labels
	{
		auto const & font = *rendering::small_font;
		for(size_t i = 0; i < shafts.size(); i++)
		{
			auto const & shaft = shafts[i];
			if(not shaft.fill_level_available)
				continue;

			auto const column_rect = get_column_rect(i);

			auto & text = font.render(shaft.title);

			SDL_Rect const dest = {
				column_rect.x + column_rect.w / 2 - text.width,
				window.y + window.h,
				text.width,
				text.height
			};
			SDL_Point const center = { dest.w, dest.h / 2 };

			SDL_RenderCopyEx(
				renderer,
				text.texture.get(),
				nullptr,
				&dest,
				-45,
				&center,
				SDL_FLIP_NONE
			);
		}
	}
}
