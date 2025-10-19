#include "application.hpp"
#include <iostream>

vv::Application::Application( const ApplicationParameters &params ):
	m_params(params)
{
}

void vv::Application::run()
{
	while(true)
	{
		for(auto &layer: m_layers)
		{
			layer->update();
		}
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