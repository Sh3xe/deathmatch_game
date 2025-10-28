#include "engine.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using namespace vv;
using dseconds = std::chrono::duration<double, std::ratio<1,1>>;

Engine::Engine( const EngineParameters &params ):
	m_params(params)
{
}

Engine::~Engine()
{
}

void Engine::run()
{
	auto current_time = std::chrono::steady_clock::now();
	double target_dt = 1.0 / m_params.target_fps;
	double current_dt = target_dt; // special case for the first dt

	while(m_running)
	{
		auto previous_time = current_time;

		// Dispatch Events
		dispatch_events();

		// Game update
		for(auto &layer: m_layers)
		{
			layer->update( current_dt );
		}

		// Rendering
		for(auto &layer: m_layers)
		{
			layer->render( current_dt );
		}

		// Tick update
		current_time = std::chrono::steady_clock::now();
		auto frame_time = current_time - previous_time;
		double delta_time_seconds = dseconds(frame_time).count();
		if( delta_time_seconds < target_dt) {
			// wait until the delta time is target_dt
			std::this_thread::sleep_for( dseconds(target_dt - delta_time_seconds) );
		}

		current_dt = std::max( target_dt, delta_time_seconds );
	}
}

void Engine::dispatch_events()
{
	// Save all the events in one array
	SDL_Event event;
	std::vector<SDL_Event> events;
	while( SDL_PollEvent(&event) )
	{
		events.push_back(event);

		if(event.type == SDL_EVENT_QUIT)
		{
			m_running = false;
		}
	}

	// and propagate them in order, one layer after the other
	for(int i = m_layers.size() - 1; i >= 0; --i)
	{
		for(SDL_Event &event: events)
			m_layers[i]->on_event(event);
	}
}

bool Engine::init_systems()
{
	if( !init_window() )
	{
		VV_ERROR("Cannot initialize SDL3");
		return false;
	}

	if( !m_graphics_sys.init( m_window ) )
	{
		VV_ERROR("Cannot initialize The graphic system");
		return false;
	}

	return true;
}

void Engine::shutdown_systems()
{
	m_graphics_sys.shutdown();
	shutdown_window();
}

void Engine::shutdown_window()
{
	SDL_DestroyWindow(m_window);
}

bool Engine::init_window()
{
	assert( m_window == nullptr ); // double initialization

	if (! SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS) )
	{
		VV_ERROR("Error when calling SDL_Init", SDL_GetError());
		return false;
	}

	SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	m_window = SDL_CreateWindow(m_params.window_title.c_str(), m_params.window_width, m_params.window_height, window_flags );

	if( m_window == nullptr ) {
		VV_ERROR("Error when calling SDL_CreateWindow: ", SDL_GetError());
		return false;
	}

	return true;
}