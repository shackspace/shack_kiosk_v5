#include "powerview.hpp"
#include "http_client.hpp"

#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>

namespace /* static */
{
	struct powernode
	{
		double time;
		double phase[3];

		double total() const { return phase[0] + phase[1] + phase[2]; }
	};

	std::mutex nodes_mutex;
	std::vector<powernode> nodes;

	[[noreturn]] static void query_thread()
	{
		using nlohmann::json;
		http_client client;

		client.set_headers({
			{ "Content-Type", "application/json" },
			{ "Access-Control-Allow-Origin", "*" },
		});

		while(true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));

			auto data = client.transfer(
				client.get,
				"http://glados.shack/siid/apps/powermeter.py?n=150"
			);
			if(not data)
				continue;
			try
			{
				auto cfg = json::parse(data->begin(), data->end());


				auto l1 = cfg["L1.Power"];
				auto l2 = cfg["L2.Power"];
				auto l3 = cfg["L3.Power"];
				auto time = cfg["Minutes ago"];

				if(l1.size() != l2.size())
					continue;
				if(l1.size() != l3.size())
					continue;
				if(l1.size() != time.size())
					continue;

				std::vector<powernode> new_nodes;
				for(size_t i = 0; i < l1.size(); i++)
				{
					auto & node = new_nodes.emplace_back();
					node.time = time[i].get<double>();
					node.phase[0] = l1[i].get<double>();
					node.phase[1] = l2[i].get<double>();
					node.phase[2] = l3[i].get<double>();
				}

				{
					std::lock_guard _ { nodes_mutex };
					nodes = new_nodes;
				}

			}
			catch(...)
			{

			}
		}
	}
}

void powerview::init()
{
	add_back_button();

	std::thread(query_thread).detach();
}

void powerview::render()
{
	gui_module::render();

	std::lock_guard _ { nodes_mutex };

	SDL_Rect const window = { 220, 20, 1040, 984 };

	SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	SDL_RenderFillRect(renderer, &window);

	auto const get_point = [&](size_t idx, double f) -> SDL_Point
	{
		return SDL_Point {
			window.x + int((window.w * idx) / nodes.size()),
			window.y + int(window.h * (1.0 - f / 3000.0)),
		};
	};

	for(size_t i = 1; i < nodes.size(); i++)
	{
		auto const & from = nodes.at(i - 1);
		auto const & to   = nodes.at(i - 0);

		for(size_t j = 0; j < 3; j++)
		{
			SDL_SetRenderDrawColor(renderer, (j==0)?255:0, (j==1)?255:0, (j==2)?255:0, 255);

			auto const p1 = get_point(i - 1, from.phase[j]);
			auto const p2 = get_point(i, to.phase[j]);
			SDL_RenderDrawLine(renderer,
				p1.x, p1.y,
				p2.x, p2.y
			);
		}
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			auto const p1 = get_point(i - 1, from.total());
			auto const p2 = get_point(i, to.total());
			SDL_RenderDrawLine(renderer,
				p1.x, p1.y,
				p2.x, p2.y
			);
		}
	}

}
