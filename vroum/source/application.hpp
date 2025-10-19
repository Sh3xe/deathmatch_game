#pragma once

#include "vv_headers.hpp"
#include "layer.hpp"

#include <SDL3/SDL.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

namespace vv
{

struct ApplicationParameters
{
	std::string window_title;
	uint32_t window_width;
	uint32_t window_height;
};

class Application
{
public:
	Application( const ApplicationParameters &params );

	bool init_modules();

	template <typename LayerType>
	void add_layer()
	{
		m_layers.push_back( std::make_unique<LayerType>() );
	}

	void run();

private:
	bool init_sdl();

private:
	ApplicationParameters m_params;
	SDL_Window *m_window = nullptr;
	std::vector<std::unique_ptr<Layer>> m_layers;
};

} // namespace vv