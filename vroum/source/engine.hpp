#pragma once

#include "vv_headers.hpp"
#include "layer.hpp"
#include "graphics/rendering_system.hpp"

#include <SDL3/SDL.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

namespace vv
{

struct EngineParameters
{
	std::string window_title = "";
	u32 window_width = 1920;
	u32 window_height = 1080;

	u32 target_fps = 30.0;
};

class Engine
{
public:
	Engine( const EngineParameters &params );
	~Engine();

	bool init_systems();
	void shutdown_systems();

	template <typename LayerType>
	void add_layer()
	{
		auto layer_ptr = std::make_unique<LayerType>();
		layer_ptr->m_app = this;
		layer_ptr->m_rend = &m_graphics_sys;

		if( layer_ptr->init() != Error::ok )
		{
			VV_ERROR("Could not initialize layer");
			return;
		}

		m_layers.push_back( std::move(layer_ptr) );
	}

	void run();

private:
	bool init_window();

	void shutdown_window();

	void dispatch_events();

private:
	RenderingSystem m_graphics_sys;
	EngineParameters m_params;
	SDL_Window *m_window = nullptr;
	std::vector<std::unique_ptr<Layer>> m_layers;
	bool m_running = true;
};

} // namespace vv