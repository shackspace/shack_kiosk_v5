#include "infoview.hpp"
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

namespace
{
	struct Muell
	{
		tm date;
		bool main_action_done;
		bool mail_sended;
	};

	protected_value<Muell> papiermuell, gelber_sack, restmuell;

	void fetch_muell(http_client & client, protected_value<Muell> & target, std::string const & uri)
	{
		auto raw = client.transfer(client.get, uri);
		if(not raw)
			return;
		try
		{
			auto const json = nlohmann::json::parse(raw->begin(), raw->end());

			Muell muell;
			std::stringstream str { std::string(json["date"]) };
			str >> std::get_time(&muell.date, "%Y-%m-%d");
			muell.mail_sended = json["mail_sended"];
			muell.main_action_done = json["main_action_done"];

			target.obtain() = std::move(muell);
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
				fetch_muell(client, gelber_sack, "http://openhab.shack/muellshack/gelber_sack");
				fetch_muell(client, papiermuell, "http://openhab.shack/muellshack/papiermuell");
				fetch_muell(client, restmuell,   "http://openhab.shack/muellshack/restmuell");
			}
			catch (...)
			{
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}
}

infoview::MuellInfo infoview::get_muell_info() const
{
	infoview::MuellInfo info;
	info.restmuell = restmuell.obtain()->date;
	info.papiermuell = papiermuell.obtain()->date;
	info.gelber_sack = gelber_sack.obtain()->date;
	return info;
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
