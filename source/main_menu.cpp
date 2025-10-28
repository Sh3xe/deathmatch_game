#include "main_menu.hpp"
#include <iostream>
#include <cmath>

MainMenu::~MainMenu()
{

}

vv::Error MainMenu::init() 
{
	return vv::Error::ok;
}

void MainMenu::shutdown() 
{

}

void MainMenu::update( double dt_sec )
{
	m_time += dt_sec;
}

void MainMenu::render( double dt_sec )
{
}

void MainMenu::on_event( const SDL_Event &event )
{
	if(event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_A)
	{
		m_app->graphic_sys().send_msg_to_thread("Hello!!");
	}
}