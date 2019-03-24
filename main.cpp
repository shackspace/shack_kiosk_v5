#include "module.hpp"
#include "modules/screensaver.hpp"
#include "modules/mainmenu.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <chrono>
#include <ctime>
#include <algorithm>

using namespace std::chrono;

SDL_Renderer * renderer;
SDL_Window * window;
SDL_Texture * home_icon;

static module * current_module;
static module * next_module;

double total_time;
double time_step;

glm::ivec2 screen_size;

std::filesystem::path resource_root;

void module::change(module * other)
{
	next_module = other;
}

static SDL_Texture * splash_icon;

struct splash
{
	double progress = 0.0;
	SDL_Point center;
};

int main()
{
	resource_root = std::filesystem::current_path() / "resources";

	srand(time(NULL));

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		die("Failed to initialize SDL: %s", SDL_GetError());
	atexit(SDL_Quit);

	if(IMG_Init(IMG_INIT_PNG) == 0)
		die("Failed to initialize SDL_image: %s", IMG_GetError());
	atexit(IMG_Quit);

	window = SDL_CreateWindow(
		"Kiosk v5.0",
		0, 0,
		1280, 1024,
		SDL_WINDOW_FULLSCREEN_DESKTOP
	);
	if(window == nullptr)
		die("Failed to create window: %s", SDL_GetError());

	renderer = SDL_CreateRenderer(
		window,
		-1, // best possible
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC
	);
	if(renderer == nullptr)
		die("Failed to create renderer: %s", SDL_GetError());

	splash_icon = IMG_LoadTexture(renderer, (resource_root / "splash.png").c_str());
	if(splash_icon == nullptr)
		die("Failed to load splash.png: %s", SDL_GetError());
	SDL_SetTextureBlendMode(splash_icon, SDL_BLENDMODE_BLEND);

	home_icon = IMG_LoadTexture(renderer, (resource_root / "icons" / "home.png").c_str());
	if(home_icon == nullptr)
		die("Failed to load home.png: %s", SDL_GetError());

	module::change<screensaver>();

	std::vector<splash> splashes;

	auto const startup = high_resolution_clock::now();
	auto last_frame = startup;
	auto last_event = startup;
	while(next_module != nullptr)
	{
		SDL_GetWindowSize(window, &screen_size.x, &screen_size.y);

		if(current_module != next_module)
		{
			if(current_module != nullptr)
				current_module->leave();
			current_module = next_module;
			if(current_module != nullptr)
				current_module->enter();
		}

		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		{
			if((ev.type == SDL_MOUSEBUTTONDOWN) or (ev.type == SDL_KEYDOWN))
				last_event = high_resolution_clock::now();

			if(ev.type == SDL_MOUSEBUTTONDOWN)
			{
				auto & sp = splashes.emplace_back();
				sp.center.x = ev.button.x;
				sp.center.y = ev.button.y;
			}
			if(ev.type == SDL_QUIT)
				next_module = nullptr;
			else
				current_module->notify(ev);
		}

		auto const now = high_resolution_clock::now();

		if((now - last_event) > seconds(15))
			module::change<screensaver>();

		total_time = duration_cast<milliseconds>(now - startup).count() / 1000.0;
		time_step = duration_cast<milliseconds>(now - last_frame).count() / 1000.0;
		last_frame = now;

		for(auto & sp : splashes)
			sp.progress += time_step;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		current_module->render();

		for(auto const & splash : splashes)
		{
			int size = 100 * pow(splash.progress, 0.5);
			SDL_Rect rekt = {
			  splash.center.x - size/2,
			  splash.center.y - size/2,
			  size, size
			};
			int x = std::max<int>(0, 255 - 255 * pow(splash.progress, 0.5));
			SDL_SetTextureAlphaMod(splash_icon, x);
			SDL_RenderCopy(
				renderer,
				splash_icon,
				nullptr,
				&rekt
			);
		}

		splashes.erase(
			std::remove_if(splashes.begin(), splashes.end(), [](splash const & s) { return s.progress >= 1.0; }),
			splashes.end()
		);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}







[[noreturn]] void __die(char const * file, int lineno, char const * msg, ...)
{
	va_list list;
	va_start(list, msg);

	fprintf(stderr, "%s:%d:", file, lineno);
	vfprintf(stderr, msg, list);
	fprintf(stderr, "\n");
	fflush(stderr);

	va_end(list);

	exit(1);
}
