#pragma once

#include <variant>
#include <memory>
#include <SDL3/SDL.h>

namespace vv
{

struct InitializeCmd
{
	InitializeCmd(SDL_Window *window): window(window) {}
	InitializeCmd() = default;

	SDL_Window *window;
};

enum class RenderCommandTypes
{
	initialize
};

struct RenderCommand
{
	RenderCommandTypes type;
	std::variant<
		std::shared_ptr<InitializeCmd>
	> data;
};

} // namespace vv