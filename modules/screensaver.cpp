#include "screensaver.hpp"
#include "mainmenu.hpp"

double constexpr PI = 3.1415;

void screensaver::init()
{
	logo = IMG_LoadTexture(
		renderer,
		(resource_root / "logo.png").c_str()
	);
	assert(logo != nullptr);

	next_effect();
}

void screensaver::next_effect()
{
	effect = rand() % 7;
}

void screensaver::enter()
{
	next_effect();
	timer = 8.0;
}

notify_result screensaver::notify(SDL_Event const & ev)
{
	if(ev.type == SDL_MOUSEBUTTONDOWN)
		activate<mainmenu>();

	if(ev.type == SDL_KEYDOWN)
		activate<mainmenu>();

	return failure;
}

void screensaver::render()
{
	timer += time_step;

	double t = timer; // 10 sekunden
	if(t >= 10.0)
	{
		t -= 10.0;
		timer -= 10.0;
		next_effect();
	}

	SDL_SetRenderDrawColor(renderer, 0,0,0,255);
	SDL_RenderClear(renderer);

	SDL_Rect rect = {0, 0, 1146, 500 };
	rect.x = (screen_size.x - rect.w) / 2;
	rect.y = (screen_size.y - rect.h) / 2;

	SDL_Point rot_point = {0, 0};
	double rot = 0;

	float a = float(rect.w) / float(rect.h);
	int flip = SDL_FLIP_NONE;
	switch(effect)
	{
		case 0: // shacky
		{
			rot_point = { rect.w / 2, rect.h / 2 };
			if(t < 1)
			{
				double ampl = sin(PI * t);
				double shacky = sin(31.34 * t);
				rot = 20 * ampl * sin(shacky);
			}
			else
			{
				rot = 0;
			}
			break;
		}

		case 1: // fall off and return
		{
			rot_point = { 0, 0 };
			if(t < 2)
			{
				rot = 80 * t * t;
				if(t >= 0.3)
					rect.y += 3000 * pow(t - 0.3, 2);
			}
			else if(t < 3)
			{
				rot = 0;
				rect.y = 3000;
			}
			else if(t < 3.25)
			{
				t -= 3;
				auto offset = rect.y + rect.h;
				rect.y = 4.0 * t * offset - rect.h;
			}
			else if(t < 3.75)
			{
				t -= 3.25;
				rot = 20 * sin(2.0 * PI * t);
			}
			else
			{
				rot = 0;
			}
			break;
		}

		case 2:
		{
			if(t < 2)
			{
				double b = pow(t,3);
				auto str = -250;
				rect.x -= a * str * b;
				rect.y -= str * b;
				rect.w += a * 2*str * b;
				rect.h += 2*str * b;
				if(rect.w < 0) rect.w = 0;
				if(rect.h < 0) rect.h = 0;
			}
			else if(t < 4)
			{
				t -= 2;
				double b = pow(2-t,3);
				auto str = -250;
				rect.x -= a * str * b;
				rect.y -= str * b;
				rect.w += a * 2*str * b;
				rect.h += 2*str * b;
				if(rect.w < 0) rect.w = 0;
				if(rect.h < 0) rect.h = 0;
			}
			break;
		}

		case 3: // wobbly
		{
			if(t < 1)
			{
				double ampl = sin(PI * t);
				double shacky = sin(20.34 * t);
				double s = 1.0 + a*0.1 * ampl * sin(shacky);
				double t = 1.0 + 0.1 * ampl * sin(shacky);

				rect.w *= s;
				rect.h /= t;

				rect.x = (screen_size.x - rect.w) / 2;
				rect.y = (screen_size.y - rect.h) / 2;
			}
			else
			{
				rot = 0;
			}
			break;
		}

		case 4: // roto?!
		{
			if(t < 2)
			{
				rect.h *= cos(1.0 * PI * t);
				if(rect.h < 0)
				{
					flip = SDL_FLIP_VERTICAL;
					rect.h = -rect.h;
				}
				rect.y = (screen_size.y - rect.h) / 2;
			}
			break;
		}

		case 5: // roto 2?!
		{
			if(t < 2)
			{
				rect.w *= cos(1.0 * PI * t);
				if(rect.w < 0)
				{
					flip = SDL_FLIP_HORIZONTAL;
					rect.w = -rect.w;
				}
				rect.x = (screen_size.x - rect.w) / 2;
			}
			break;
		}

		case 6: // slicer
		{
			if(t < 1)
			{
				double ampl = sin(PI * t);
				double shacky = sin(31.34 * t);

				for(int i = 0; i < rect.h; i++)
				{
					SDL_Rect dstrect = rect;
					dstrect.y += i;
					dstrect.h = 1;
					dstrect.x += 30.0 * ampl * sin(shacky) * sin(0.035 * i);

					SDL_Rect srcrect = dstrect;
					srcrect.x = 0;
					srcrect.y = i;

					SDL_RenderCopy(
						renderer,
						logo,
						&srcrect,
						&dstrect
					);
				}
				return; // IMPORTANT
			}
			else
			{
				break;
			}
		}
	}

	SDL_RenderCopyEx(
		renderer,
		logo,
		nullptr,
		&rect,
		rot,
		&rot_point,
		SDL_RendererFlip(flip)
	);
}
