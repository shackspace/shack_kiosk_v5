#include "mainmenu.hpp"
#include "widgets/button.hpp"
#include "modules/lightroom.hpp"
#include "modules/tramview.hpp"
#include "modules/powerview.hpp"
#include "modules/mateview.hpp"
#include "modules/infoview.hpp"
#include "modules/eventsview.hpp"
#include "http_client.hpp"
#include "rendering.hpp"
#include "rect_tools.hpp"
#include "protected_value.hpp"

#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>
#include <atomic>

#include <iostream>

namespace
{
	using nlohmann::json;

	int constexpr item_padding = 50;

	bool is_open = false;
	protected_value<std::string> keyholder { "???" };

	struct VolumioInfo
	{
		bool playing;
		std::string artist;
		std::string song;
		std::string album;
		std::string albumart_uri;
		std::vector<std::byte> coverdata;
		bool coverart_dirty = false;
	};

	protected_value<VolumioInfo> volumio;

	bool update_volumio(http_client & client)
	{
		auto data = client.transfer(
			client.get,
			"http://lounge.volumio.shack/api/v1/getstate"
		);
		if(not data)
			return false;
		try
		{
			// {
			//  "status":"play",
			//  "title":"Bliss on Mushrooms",
			//  "artist":"Infected Mushroom, Bliss, Miyavi",
			//  "album":"Head of NASA and the 2 Amish Boys",
			//  "albumart":"https://i.scdn.co/image/770fcf37b83ad71be38fdfb39ac1278251fddca2",
			//  "uri":"spotify:track:6rCcJf8p7z9QDlfzmBVSVl",
			//  "trackType":"spotify",
			//  "seek":68000,
			//  "duration":571,
			//  "samplerate":"44.1 KHz",
			//  "bitdepth":"16 bit",
			//  "channels":2,
			//  "random":false,
			//  "repeat":false,
			//  "repeatSingle":false,
			//  "consume":false,
			//  "volume":88,
			//  "mute":false,
			//  "disableVolumeControl":false,
			//  "stream":false,
			//  "updatedb":false,
			//  "volatile":true,
			//  "service":"volspotconnect2"
			// }
			auto cfg = json::parse(data->begin(), data->end());

			bool changed_albumart = false;

			VolumioInfo info;
			info.playing = cfg["status"] == "play";
			info.song    = cfg.value("title", "-");
			info.artist  = cfg.value("artist", "-");

			if(cfg.at("album") != json{})
				info.album   = cfg.value("album", "-");
			else
				info.album = "";

			std::string uri = cfg.value("albumart", "");
			changed_albumart  =(info.albumart_uri != uri);
			info.albumart_uri = uri;

			volumio.obtain() = info;

			return changed_albumart;
		}
		catch(...)
		{

		}
		return false;
	}

	void update_albumart(http_client & client)
	{
		std::string uri = volumio.obtain()->albumart_uri;
		if(not uri.empty() and (uri.at(0) == '/'))
		{
			uri = "http://lounge.volumio.shack" + uri;
		}

		auto data = client.transfer(
			client.get,
			uri
		);

		auto info = volumio.obtain();
		if(not data)
			info->coverdata.clear();
		else
			info->coverdata = *data;
		info->coverart_dirty = true;
	}

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

			is_open = cfg["status"] == "open";
			keyholder.obtain() = cfg["keyholder"];
		}
		catch(...)
		{

		}
	}

	[[noreturn]] void loop()
	{
		http_client client;
		client.set_headers({
			{ "Content-Type", "application/json" },
			{ "Access-Control-Allow-Origin", "*" },
		});

		while(true)
		{
			update_keyholder(client);

			if(update_volumio(client))
			{
				update_albumart(client);
			}

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
	center_widgets.push_back(set<mateview>(add<button>(), *col++, "bottle-wine.png"));
	center_widgets.push_back(set<infoview>(add<button>(), *col++, "information.png"));

	center_widgets.push_back(set<powerview>(add<button>(), *col++, "flash.png"));

	// center_widgets.push_back(songbutton = set<mainmenu>(add<button>(), *col++, "volume-high.png"));

	// center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "cellphone-key.png"));
	// center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "alert.png"));
	center_widgets.push_back(set<eventsview>(add<button>(), *col++, "calendar-month.png"));

	// volumio_albumart_none = songbutton->icon;

	key_icon = IMG_LoadTexture(renderer, (resource_root / "icons" / "key-variant.png" ).c_str());
	power_icon = IMG_LoadTexture(renderer, (resource_root / "icons" / "flash.png" ).c_str());
	skull_icon = IMG_LoadTexture(renderer, (resource_root / "icons" / "skull.png" ).c_str());

	volumio_icon_song  = IMG_LoadTexture(renderer, (resource_root / "icons" / "volumio.png" ).c_str());
	volumio_icon_artist  = IMG_LoadTexture(renderer, (resource_root / "icons" / "artist.png" ).c_str());
	volumio_icon_album  = IMG_LoadTexture(renderer, (resource_root / "icons" / "album.png" ).c_str());
	volumio_play  = IMG_LoadTexture(renderer, (resource_root / "icons" / "play.png" ).c_str());
	volumio_pause = IMG_LoadTexture(renderer, (resource_root / "icons" / "pause.png" ).c_str());
	volumio_next = IMG_LoadTexture(renderer, (resource_root / "icons" / "skip-next.png" ).c_str());

	auto * nextbutton = add<button>();
	nextbutton->bounds = { 1280 - 90, 10, 80, 80 };
	nextbutton->icon = volumio_next;
	nextbutton->on_click = [&]() {
		std::thread([]()
		{
			http_client client;
			client.set_headers({
				{ "Content-Type", "application/json" },
				{ "Access-Control-Allow-Origin", "*" },
			});
			client.transfer(
				client.get,
				"http://lounge.volumio.shack/api/v1/commands/?cmd=next"
			);
		}).detach();
	};


	playpausebutton = add<button>();
	playpausebutton->bounds = { 1280 - 90 - 100, 10, 80, 80 };
	playpausebutton->icon = volumio_play;
	playpausebutton->on_click = []() {
		std::thread([]()
		{
			http_client client;
			client.set_headers({
				{ "Content-Type", "application/json" },
				{ "Access-Control-Allow-Origin", "*" },
			});

			std::string method = volumio.obtain()->playing ? "play" : "pause";

			client.transfer(
				client.get,
				"http://lounge.volumio.shack/api/v1/commands/?cmd=" + method
			);
		}).detach();
	};

	std::thread(loop).detach();
}

void mainmenu::layout()
{
	auto const center_off = glm::ivec2(0, 100);
	auto const center_size = screen_size - 2 * center_off;

	// SDL_Rect const top_bar = { 0, 0, screen_size.x, center_off.y };
	// SDL_Rect const bottom_bar = { 0, screen_size.y - center_off.y - 1, screen_size.x, center_off.y };

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
}

void mainmenu::render()
{
	auto timestamp = std::time(nullptr);
	std::tm const * const clock = std::localtime(&timestamp);

	module_cycle_progress += time_step;
	while(module_cycle_progress >= 4.0f)
	{
		module_cycle_progress -= 4.0f;
		module_cycle++;
	}
	modules_cycling = (module_cycle_progress <= 1.0f);


	bool volumio_playing;
	{
		auto info = volumio.obtain();

		if(info->playing)
			playpausebutton->icon = volumio_pause;
		else
			playpausebutton->icon = volumio_play;

		if(info->coverart_dirty)
		{
			if(volumio_albumart != nullptr)
				SDL_DestroyTexture(volumio_albumart);
			volumio_albumart = nullptr;
			if(not info->coverdata.empty())
			{
				volumio_albumart = IMG_LoadTexture_RW(
					renderer,
					SDL_RWFromMem(info->coverdata.data(), info->coverdata.size()),
					1
				);
			}
		}
		info->coverart_dirty = false;
		volumio_playing = info->playing;
	}

//	if(volumio_playing and (volumio_albumart != nullptr))
//	{
//			songbutton->background = volumio_albumart;
//			if(clock->tm_sec % 2)
//				songbutton->icon_tint = { 0xFF, 0xFF, 0xFF, 255 };
//			else
//				songbutton->icon_tint = { 0x00, 0x00, 0x00, 0xFF };
//	}
//	else
//	{
//		songbutton->background = nullptr;
//		songbutton->icon_tint = { 0x00, 0x00, 0x00, 0xFF };
//	}

	auto const center_off = glm::ivec2(0, 100);
	auto const center_size = screen_size - 2 * center_off;

	SDL_Rect const top_bar = { 0, 0, screen_size.x, center_off.y };
	SDL_Rect const bottom_bar = { 0, screen_size.y - center_off.y - 1, screen_size.x, center_off.y };


	SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	SDL_RenderFillRect(renderer, &top_bar);

	SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	SDL_RenderFillRect(renderer, &bottom_bar);

	gui_module::render();

	std::array bottom_modules =
	{
	  SDL_Rect {  0 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  SDL_Rect {  1 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  SDL_Rect {  2 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  SDL_Rect {  3 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	  SDL_Rect { -1 * bottom_bar.w / 3, bottom_bar.y, bottom_bar.w / 3, bottom_bar.h },
	};
	if(modules_cycling)
	{
		for(auto & mod : bottom_modules)
			mod.x += int((1.0 - module_cycle_progress) * mod.w + 0.5);
	}

	// Volumio Top Control
	{
		std::string text;
		SDL_Texture * icon;
		{
			auto info = volumio.obtain();
			switch((clock->tm_sec / 4) % 3)
			{
				case 0: text = info->song;   icon = volumio_icon_song; break;
				case 1: text = info->album;  icon = volumio_icon_album; break;
				case 2: text = info->artist; icon = volumio_icon_artist; break;
				default:
					abort();
			}
		}

		SDL_Rect left = top_bar;
		left.w = left.h;
		left = add_margin(left, 10);

		SDL_RenderCopy(
			renderer,
			icon,
			nullptr,
			&left
		);

		rendering::big_font->render(
			top_bar,
			text
		);
	}

	for(size_t i = 0; i < bottom_modules.size(); i++)
	{
		SDL_SetRenderDrawColor(renderer, 32, 32, 32, 0xFF);
		SDL_RenderFillRect(
			renderer,
			&bottom_modules[i]
		);

		SDL_SetRenderDrawColor(renderer, 8, 8, 8, 0xFF);
		SDL_RenderDrawRect(
			renderer,
			&bottom_modules[i]
		);
	}

	using module_renderer = void (mainmenu::*)(SDL_Rect);
	module_renderer renderers[] =
	{
		&mainmenu::render_power_module,
		&mainmenu::render_event_module,
		&mainmenu::render_trash_module,
		&mainmenu::render_keyholder_module,
	  &mainmenu::render_clock_module,
	};

	auto const render_module = [&](int index, size_t rect_id)
	{
		while(index < 0)
			index += std::size(renderers);
		auto ren = renderers[size_t(index + module_cycle) % std::size(renderers)];

		SDL_RenderSetClipRect(renderer, &bottom_modules[rect_id]);

		(this->*ren)(bottom_modules[rect_id]);
	};

	for(int i = 0; i < 3; i++)
	{
		render_module(i, size_t(i));
	}
	render_module( 3, 3);
	render_module(-1, 4);

	SDL_RenderSetClipRect(renderer, nullptr);
}

void mainmenu::render_power_module(SDL_Rect module_rect)
{
	SDL_Rect left = module_rect;
	left.w = left.h;
	left = add_margin(left, 10);

	SDL_Rect name = module_rect;
	name.x += module_rect.h;
	name.w -= module_rect.h;

	auto const power = module::get<powerview>()->total_power;

	if(power >= 0)
	{
		SDL_RenderCopy(
			renderer,
			power_icon,
			nullptr,
			&left
		);

		rendering::big_font->render(
			module_rect,
			std::to_string(int(power)) + " W"
		);
	}
	else
	{
		SDL_RenderCopy(
			renderer,
			skull_icon,
			nullptr,
			&left
		);

		rendering::big_font->render(
			module_rect,
			"GlaDOS",
			FontRenderer::Middle | FontRenderer::Center,
			{ 0xFF, 0x00, 0x00, 0xFF }
		);
	}
}

void mainmenu::render_keyholder_module(SDL_Rect module_rect)
{
	SDL_Rect left = module_rect;
	left.w = left.h;
	left = add_margin(left, 10);

	SDL_Rect name = module_rect;
	name.x += module_rect.h;
	name.w -= module_rect.h;

	SDL_RenderCopy(
		renderer,
		key_icon,
		nullptr,
		&left
	);
	{
		rendering::big_font->render(
			module_rect,
			*keyholder.obtain()
		);
	}
}

void mainmenu::render_trash_module(SDL_Rect module_rect)
{
	rendering::big_font->render(
		module_rect,
		"Müll-Info"
	);
}

void mainmenu::render_event_module(SDL_Rect module_rect)
{
	if(auto ev = module::get<eventsview>()->current_event(); ev)
	{
		rendering::big_font->render(
			module_rect,
			ev->title
		);
	}
	else
	{
		rendering::big_font->render(
			module_rect,
			"Kein Event"
		);
	}
}

void mainmenu::render_clock_module(SDL_Rect module_rect)
{
	auto const now_t = std::time(nullptr);
	std::tm const now = *std::localtime(&now_t);

	char buffer[128];
	snprintf(buffer, sizeof buffer, "%02d:%02d:%02d", now.tm_hour, now.tm_min, now.tm_sec);

	rendering::big_font->render(
		module_rect,
		buffer
	);
}
