#include "eventsview.hpp"
#include "http_client.hpp"
#include "rendering.hpp"
#include "protected_value.hpp"

#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>
#include <atomic>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <iostream>

namespace
{
	struct Event
	{
		std::string title;
		std::time_t start, end;
		std::string room;
		bool is_series;
	};

	protected_value<std::vector<Event>> events;

	void fetch(http_client & client)
	{
		auto raw = client.transfer(
			client.get,
			"https://events-api.shackspace.de/events/"
		);
		if(not raw)
		{
			return;
		}
		try
		{
			auto const json = nlohmann::json::parse(raw->begin(), raw->end());
			std::vector<Event> list;

			for(auto const & val : json)
			{
				auto & event = list.emplace_back();

				std::stringstream date_start { std::string(val["start"]) };
				std::stringstream date_end   { std::string(val["end"]) };

				std::tm tm_start {};
				std::tm tm_end {};
				// 2018-04-09T15:06:25.338943+02:00

				date_start >> std::get_time(&tm_start, "%Y-%m-%dT%H:%M:%S");
				date_end >> std::get_time(&tm_end, "%Y-%m-%dT%H:%M:%S");

				event.title = val["name"];
				event.start = std::mktime(&tm_start);
				event.end = std::mktime(&tm_end);
			}

			std::time_t now_t;
			{
				auto pos_t = std::time(nullptr);
				auto pos = *std::gmtime(&pos_t);
				pos.tm_min = 0;
				pos.tm_hour = 0;
				pos.tm_sec = 0;
				pos.tm_isdst = 0;
				now_t = std::mktime(&pos);
				assert(now_t != -1);
			}

			// erase all events in the past
			list.erase(std::remove_if(list.begin(), list.end(), [&](Event const & e) {
				// auto const day = *std::localtime(&e.start);
				// return (day.tm_yday < now.tm_yday) and (day.tm_year <= now.tm_yday);
				return std::difftime(e.end, now_t) < 0;
			}), list.end());

			// erase all events in the "far" future (1 week)
//			list.erase(std::remove_if(list.begin(), list.end(), [&](Event const & e) {
//				return std::difftime(e.start, now_t) >= (3600 * 24 * 7);
//			}), list.end());

			// sort list by start of event
			std::sort(list.begin(), list.end(), [](Event const & e1, Event const & e2)
			{
				return std::difftime(e1.start, e2.start) < 0;
			});

			// limit the list to 20 entries
			if(list.size() > 10)
				list.resize(10);

			*events.obtain() = std::move(list);
		}
		catch(...)
		{

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
				fetch(client);
			}
			catch (...)
			{
			}
			std::this_thread::sleep_for(std::chrono::minutes(10));
		}
	}
}

void eventsview::init()
{
	add_back_button();
	std::thread(task).detach();
}

void eventsview::render()
{
	gui_module::render();

	auto const & font = *rendering::small_font;

	SDL_Rect rect = { 220, 10, 1050, 50 };
	bool odd = false;
	for(auto const & ev : *events.obtain())
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, odd ? 0x10 : 0x20);
		SDL_RenderFillRect(renderer, &rect);

		auto const tm = *std::localtime(&ev.start);
		auto const duration = std::difftime(ev.end, ev.start);
		char buffer[256];
		snprintf(buffer, sizeof buffer, "%02d.%02d.%04d %02d:%02d", tm.tm_mday, tm.tm_mon, tm.tm_year, tm.tm_hour, tm.tm_min);

		std::string duration_text;
		if(duration < 3600)
			duration_text = std::to_string(int(std::ceil(duration / 60))) + " Min.";
		else if(duration < 24 * 3600)
			duration_text = std::to_string(int(std::ceil(duration / 3600))) + " Std.";
		else
			duration_text = std::to_string(int(std::ceil(duration / (3600 * 24)))) + " Tage";

		auto padded_rect = rect;
		padded_rect.x += 10;
		padded_rect.w -= 20;

		font.render(
			padded_rect,
			ev.title + " (" + duration_text + ")",
			font.Left | font.Middle
		);

		font.render(
			padded_rect,
			buffer,
			font.Right | font.Middle
		);

		rect.y += rect.h;
		odd = !odd;
	}
}
