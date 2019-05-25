#ifndef RECT_TOOLS_HPP
#define RECT_TOOLS_HPP

#include <SDL.h>

SDL_Rect inline add_margin(SDL_Rect rect, int left, int top, int right, int bottom)
{
	rect.x += left;
	rect.y += top;
	rect.w -= (left + right);
	rect.h -= (top + bottom);
	return rect;
}

SDL_Rect inline add_margin(SDL_Rect rect, int margin_h, int margin_v)
{
	return add_margin(rect, margin_h, margin_v, margin_h, margin_v);
}

SDL_Rect inline add_margin(SDL_Rect rect, int margin_all)
{
	return add_margin(rect, margin_all, margin_all);
}

auto inline split_horizontal(SDL_Rect rect, int diff)
{
	SDL_Rect left, right;
	left = rect;
	right = rect;
	left.w = diff;
	right.x += diff;
	right.w -= diff;
	struct { SDL_Rect left, right; } result { left, right };
	return result;
}

auto inline split_vertical(SDL_Rect rect, int diff)
{
	SDL_Rect top, bottom;
	top = rect;
	bottom = rect;
	top.h = diff;
	bottom.y += diff;
	bottom.h -= diff;
	struct { SDL_Rect top, bottom; } result { top, bottom };
	return result;
}

#endif // RECT_TOOLS_HPP
