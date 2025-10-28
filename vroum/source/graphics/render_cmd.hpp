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

struct ShutdownCmd
{
	// empty
};

enum class RenderCmdType
{
	initialize, shutdown
};

struct RenderCmd
{
	using RenderCmdVariant = std::variant<
		std::shared_ptr<InitializeCmd>,
		ShutdownCmd
	>;

	RenderCmd() = default;

	RenderCmd(const RenderCmdType &type, const RenderCmdVariant &data ):
		type(type), data(data) {}

	RenderCmdType type;
	RenderCmdVariant data;
};

} // namespace vv