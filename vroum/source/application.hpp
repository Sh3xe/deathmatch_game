#pragma once

#include "vv_headers.hpp"
#include "layer.hpp"
#include "graphics/graphics_system.hpp"

#include <SDL3/SDL.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

namespace vv
{

struct ApplicationParameters
{
	std::string window_title = "";
	uint32_t window_width = 1920;
	uint32_t window_height = 1080;

	uint32_t target_fps = 30.0;
};

class Application
{
public:
	Application( const ApplicationParameters &params );
	~Application();

	bool init_systems();
	void shutdown_systems();

	template <typename LayerType>
	void add_layer()
	{
		auto layer_ptr = std::make_unique<LayerType>();
		layer_ptr->m_app = this;
		layer_ptr->m_rend = &m_graphics_sys;
		m_layers.push_back( std::move(layer_ptr) );
	}

	void run();

	GraphicsSystem &graphic_sys() { return m_graphics_sys; }

private:
	bool init_window();
	void shutdown_window();

	void dispatch_events();

private:
	GraphicsSystem m_graphics_sys;
	ApplicationParameters m_params;
	SDL_Window *m_window = nullptr;
	std::vector<std::unique_ptr<Layer>> m_layers;
	bool m_running = true;
};

} // namespace vv