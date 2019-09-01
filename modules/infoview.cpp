#include "infoview.hpp"
#include "http_client.hpp"
#include "rendering.hpp"
#include "protected_value.hpp"
#include "rect_tools.hpp"

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

			// initializing time values to prevent std::mktime() from chaotically changing the date
		 	muell.date.tm_sec = 0;
			muell.date.tm_min = 0;
			muell.date.tm_hour = 0;

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

	static bool do_alert_muell(tm const & date)
	{
		auto const termin = std::mktime(&const_cast<tm&>(date) );
		return std::difftime(termin, std::time(nullptr)) > -(3600 * 24 * 1.5);
	}
}

infoview::MuellInfo infoview::get_muell_info() const
{
	infoview::MuellInfo info;
	info.restmuell = restmuell.obtain()->date;
	info.papiermuell = papiermuell.obtain()->date;
	info.gelber_sack = gelber_sack.obtain()->date;

	info.warn_restmuell = do_alert_muell(info.restmuell);
	info.warn_papiermuell = do_alert_muell(info.papiermuell);
	info.warn_gelber_sack = do_alert_muell(info.gelber_sack);

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

	auto const render_muellinfo = [&](SDL_Rect target, std::string const & title, Muell const & muell)
	{
		auto const [ left_half, right_half ] = split_horizontal(target, target.w / 2);

		auto const [ title_pos, date_pos ] = split_horizontal(left_half, left_half.w / 2);
		auto const [ done_pos, mail_pos ] = split_horizontal(right_half, left_half.w / 2);

		rendering::small_font->render(
			title_pos,
			title,
			FontRenderer::Left | FontRenderer::Middle
		);

		auto const alert = do_alert_muell(muell.date);

		{
			char buffer[256];
			snprintf(
				buffer, sizeof buffer,
				"%02d.%02d.%02d",
				muell.date.tm_mday, 1 + muell.date.tm_mon, 1900 + muell.date.tm_year
			);
			rendering::small_font->render(
				date_pos,
				buffer,
				FontRenderer::Left | FontRenderer::Middle
			);
		}

		if(alert)
		{
			rendering::small_font->render(
				done_pos,
				muell.main_action_done ? "Erledigt" : "Rausbringen!",
				FontRenderer::Left | FontRenderer::Middle
			);

			rendering::small_font->render(
				mail_pos,
				muell.mail_sended ? "Mail erledigt" : "Mail ausstehend",
				FontRenderer::Right | FontRenderer::Middle
			);
		}
	};

	render_muellinfo({ 240,  30, 1030, 50 }, "Restmüll",    *restmuell.obtain());
	render_muellinfo({ 240,  80, 1030, 50 }, "Papiermüll",  *papiermuell.obtain());
	render_muellinfo({ 240, 130, 1030, 50 }, "Gelber Sack", *gelber_sack.obtain());
}
