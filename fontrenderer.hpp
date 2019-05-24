#ifndef FONTRENDERER_HPP
#define FONTRENDERER_HPP

#include <SDL_ttf.h>
#include <map>
#include <string>
#include <memory>

struct FontRenderer
{
	enum Align
	{
		Top = 0,
		Left = 0,
		Middle = 1,
		Center = 2,
		Right = 4,
		Bottom = 8,
	};

	using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

	struct Text
	{
		TexturePtr texture;
		int width, height;
		size_t ttl = 1;
		size_t last_use;

		explicit Text(TexturePtr && tex);
	};

	SDL_Renderer * renderer;
	std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font;
	mutable std::map<std::string, Text> cache;
	size_t generation;

	explicit FontRenderer(SDL_Renderer * renderer, TTF_Font * font);

	Text const & render(std::string const & what) const;

	void render(SDL_Rect const & target, std::string const & what, int align = Middle | Center, SDL_Color const & color = { 0xFF, 0xFF, 0xFF, 0xFF }) const;

	void collect_garbage();

	int height() const {
		return TTF_FontHeight(font.get());
	}
};

#endif // FONTRENDERER_HPP
