#include "mainmenu.hpp"
#include "widgets/button.hpp"
#include "modules/lightroom.hpp"

namespace
{
	int constexpr item_padding = 50;
}

template<typename Target>
static button * set(button * b, SDL_Color color, char const * file)
{
	b->color = color;
	b->icon = IMG_LoadTexture(renderer, (resource_root / "icons" / file).c_str());
	b->on_click = []() {
		module::change<Target>();
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
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "tram.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "bottle-wine.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "information.png"));

	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "flash.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "volume-high.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "cellphone-key.png"));
	center_widgets.push_back(set<mainmenu>(add<button>(), *col++, "alert.png"));
}

void mainmenu::layout()
{
	auto const center_off = glm::ivec2(0, 100);
	auto const center_size = screen_size - 2 * center_off;

	SDL_Rect top_bar = { 0, 0, screen_size.x, center_off.y };
	SDL_Rect bottom_bar = { 0, screen_size.y - center_off.y - 1, screen_size.x, center_off.y };

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
