#include "tramview.hpp"
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

namespace
{
	using std::chrono::system_clock;

	std::atomic_bool data_available;

	struct Departure
	{
		enum Direction { UnknownDirection = 0, ToCity = 1, FromCity = 2 };
		enum Route { UnknownRoute = 0, U4 = 1, U9 = 2, N1 = 3, N2= 4, N6 = 5, N7 = 6 };

		Direction direction;
		Route route;
		std::time_t departure;
		std::string target;
	};

	protected_value<std::vector<Departure>> departures;

	void fetch(http_client & client)
	{
		auto raw = client.transfer(
			client.get,
			"https://efa-api.asw.io/api/v1/station/5000082/departures/?format=json"
		);
		if(not raw)
		{
			data_available = false;
			return;
		}
		try
		{
			auto const json = nlohmann::json::parse(raw->begin(), raw->end());

			std::vector<Departure> data;
			for(auto const & src : json)
			{
				Departure & dst = data.emplace_back();

				if(src["number"] == "N1")
					dst.route = dst.N1;
				else if(src["number"] == "N2")
					dst.route = dst.N2;
				else if(src["number"] == "N6")
					dst.route = dst.N6;
				else if(src["number"] == "N7")
					dst.route = dst.N7;
				else if(src["number"] == "U4")
					dst.route = dst.U4;
				else if(src["number"] == "U9")
					dst.route = dst.U9;
				else
					dst.route = dst.UnknownRoute;

				dst.target = src["direction"];

				if(src["direction"] == "Untertürkheim Bf")
					dst.direction = dst.FromCity;
				else if(src["direction"] == "Hedelfingen")
					dst.direction = dst.FromCity;
				else if(src["direction"] == "Hölderlinplatz")
					dst.direction = dst.ToCity;
				else if (src["direction"] == "Heslach Vogelrain")
					dst.direction = dst.ToCity;
				else
					dst.direction = dst.UnknownDirection;

				std::time_t t = std::time(nullptr);
				tm time = *std::localtime(&t);
				{
					auto json_time = src["departureTime"];

					time.tm_sec = 0;
					time.tm_min  = std::stol(std::string(json_time["minute"]));
					time.tm_hour = std::stol(std::string(json_time["hour"]));
					time.tm_mday = std::stol(std::string(json_time["day"]));
					time.tm_mon  = std::stol(std::string(json_time["month"])) - 1;
					time.tm_year = std::stol(std::string(json_time["year"])) - 1900;
					// time.tm_wday = std::stol((std::string)json_time["weekday"]);
				}
				dst.departure = std::mktime(&time);
				if(dst.departure == -1)
					perror("failed to convert time");
			}
			std::sort(data.begin(), data.end(), [](Departure const & a, Departure const & b)
			{
				return a.departure < b.departure;
			});

			*departures.obtain() = std::move(data);
			data_available = true;
		}
		catch(...)
		{
			data_available = false;
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
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}
}

void tramview::init()
{
	add_back_button();

	background = IMG_LoadTexture(renderer, (resource_root / "tram" / "background.png").c_str());
	assert(background != nullptr);

	route_icons[0] = IMG_LoadTexture(renderer, (resource_root / "tram" / "U4.png").c_str());
	route_icons[1] = IMG_LoadTexture(renderer, (resource_root / "tram" / "U9.png").c_str());
	route_icons[2] = IMG_LoadTexture(renderer, (resource_root / "tram" / "N1.png").c_str());
	route_icons[3] = IMG_LoadTexture(renderer, (resource_root / "tram" / "N2.png").c_str());
	route_icons[4] = IMG_LoadTexture(renderer, (resource_root / "tram" / "N6.png").c_str());
	route_icons[5] = IMG_LoadTexture(renderer, (resource_root / "tram" / "N7.png").c_str());

	std::thread(task).detach();
}

void tramview::render()
{
	SDL_RenderCopy(renderer, background, nullptr, nullptr);

	gui_module::render();

	struct Lists
	{
		SDL_Rect background;
		SDL_Rect icon;
		SDL_Rect textfield;

		int counter = 0;

		explicit Lists(SDL_Rect target)
		{
			background = target;
			target.x += 5;
			target.y += 5;
			target.w -= 10;
			target.h -= 10;

			icon = target;
			icon.w = 2 * target.h;

			target.x += icon.w + 5;
			target.w -= icon.w + 5;

			textfield = target;
		}

		void advance()
		{
			auto const delta = background.h;
			background.y += delta;
			icon.y += delta;
			textfield.y += delta;
		}
	};

	std::array<Lists, 2> lists =
	{
	  Lists { { 620, 300, 600, 50 } }, // to city
	  Lists { { 90, 680, 600, 50 } }, // from city
	};

	for(auto const & dep : *departures.obtain())
	{
		auto const time_diff= std::difftime(dep.departure, std::time(nullptr));
		if(time_diff < 0)
			continue;

		Lists * list;
		switch(dep.direction)
		{
			case Departure::ToCity:
				list = &lists[0];
				break;
			case Departure::FromCity:
				list = &lists[1];
				break;
			default:
				continue;
		}
		if(list->counter >= 4)
			continue;

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xC0);
		SDL_RenderFillRect(renderer, &list->background);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderCopy(
			renderer,
			route_icons[dep.route - 1],
			nullptr,
			&list->icon
		);

		rendering::small_font->render(
			list->textfield,
			dep.target,
		  FontRenderer::Left | FontRenderer::Middle,
			{ 0,0,0,255 }
		);

		Uint8 r = 0;
		if(time_diff <= 360) // JETZT ABER SCHNELL
			r = 160 + 95 * sin(4.0 * total_time);

		rendering::small_font->render(
			list->textfield,
			std::to_string(int(std::floor(time_diff / 60.0))) + " Min.",
		  FontRenderer::Right | FontRenderer::Middle,
			{ r, 0, 0, 255 }
		);

		list->advance();
		list->counter++;
	}
}
