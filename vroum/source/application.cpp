#include "application.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using dseconds = std::chrono::duration<double, std::ratio<1,1>>;

vv::Application::Application( const ApplicationParameters &params ):
	m_params(params)
{
}

void vv::Application::run()
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

void vv::Application::dispatch_events()
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

bool vv::Application::init_modules()
{
	if( !init_sdl() )
	{
		std::cout << "Cannot initialize SDL3" << std::endl;
		return false;
	}

	return true;
}

bool vv::Application::init_sdl()
{
	assert( m_window == nullptr ); // double initialization

	if (! SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) )
	{
		std::cout << "Error when calling SDL_Init" << SDL_GetError() << std::endl;
		return false;
	}

	m_window = SDL_CreateWindow(m_params.window_title.c_str(), m_params.window_width, m_params.window_height, 0 );

	if( m_window == nullptr ) {
		std::cout << "Error when calling SDL_CreateWindow: " << SDL_GetError() << std::endl;
		return false;
	}

	return true;
}