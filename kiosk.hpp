#ifndef KIOSK_HPP
#define KIOSK_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <filesystem>

[[noreturn]] void __die(char const * file, int lineno, char const * msg, ...);

#define die(...) __die(__FILE__, __LINE__, __VA_ARGS__)

extern SDL_Renderer * renderer;
extern SDL_Window * window;

extern double time_step;  // delta time in seconds
extern double total_time; // total time in seconds since start

extern glm::ivec2 screen_size; // screen size in pixels

extern std::filesystem::path resource_root; // root folder for all resources

#endif // KIOSK_HPP
