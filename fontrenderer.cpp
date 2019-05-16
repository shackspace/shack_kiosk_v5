#include "fontrenderer.hpp"
#include <cassert>

FontRenderer::FontRenderer(SDL_Renderer * renderer, TTF_Font * font) :
  renderer{ renderer },
  font { font, TTF_CloseFont },
  cache { },
  generation { 0 }
{

}

FontRenderer::Text & FontRenderer::render(const std::string & what)
{
	auto str = what.empty() ? " " : what;
	if(auto it = cache.find(what); it != cache.end())
	{
		if(it->second.last_use != generation)
		{
			it->second.ttl ++;
			it->second.last_use = generation;
		}
		return it->second;
	}
	else
	{
		std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface {
			TTF_RenderUTF8_Blended(font.get(), str.c_str(), { 0xFF, 0xFF, 0xFF, 0xFF }),
			SDL_FreeSurface
		};
		assert(surface);

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture {
			SDL_CreateTextureFromSurface(renderer, surface.get() ) ,
			SDL_DestroyTexture
		};
		assert(texture);

		auto [ new_it, emplaced ] = cache.emplace(what, std::move(texture));
		assert(emplaced);
		new_it->second.width = surface->w;
		new_it->second.height = surface->h;
		return new_it->second;
	}
}

void FontRenderer::render(const SDL_Rect & target, const std::string & what, int align, SDL_Color const & color)
{
	Text & str = render(what);

	SDL_Rect dst, src;
	dst.w = std::min(str.width, target.w);
	dst.h = std::min(str.height, target.h);

	if(align & Right)
		dst.x = target.x + target.w - dst.w;
	else if(align & Center)
		dst.x = target.x + (target.w - dst.w) / 2;
	else
		dst.x = target.x;

	if(align & Bottom)
		dst.y = target.y + target.h - dst.h;
	else if(align & Middle)
		dst.y = target.y + (target.h - dst.h) / 2;
	else
		dst.y = target.y;

	src.w = dst.w;
	src.h = dst.h;
	src.x = (str.width - dst.w) / 2;
	src.y = (str.height - dst.h) / 2;

	SDL_SetTextureColorMod(str.texture.get(), color.r, color.g, color.b);

	SDL_RenderCopy(
		renderer,
		str.texture.get(),
		&src,
		&dst
	);
}

void FontRenderer::collect_garbage()
{
	generation += 1;
}

FontRenderer::Text::Text(FontRenderer::TexturePtr && tex) :
	texture(std::move(tex)),
	ttl(1),
	last_use(~0U)
{

}
