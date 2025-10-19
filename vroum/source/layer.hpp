#pragma once

#include <SDL3/SDL_events.h>

class Application;

namespace vv
{

class Layer
{
public:
	virtual ~Layer() {}

	virtual void render( double dt_sec ) = 0;

	virtual void update( double dt_sec ) = 0;

	virtual void on_event( const SDL_Event &event ) = 0;

private:
	Application *m_app;
};

} // namespace vv