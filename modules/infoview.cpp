#include "infoview.hpp"
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
	void fetch(http_client & client)
	{
		auto raw = client.transfer(
			client.get,
			""
		);
		if(not raw)
		{
			return;
		}
		try
		{
			auto const json = nlohmann::json::parse(raw->begin(), raw->end());

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

			}
			catch (...)
			{
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}
}

void infoview::init()
{
	add_back_button();
	std::thread(task).detach();
}

void infoview::render()
{
	gui_module::render();



}
