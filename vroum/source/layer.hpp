#pragma once

#include <SDL3/SDL_events.h>
#include "vv_errors.hpp"

namespace vv
{

class Engine;
class RenderingSystem;

class Layer
{
public:
	friend class Engine;

	virtual ~Layer() {}

	virtual Error init() = 0;

	virtual void shutdown() = 0;

	virtual void render( double dt_sec ) = 0;

	virtual void update( double dt_sec ) = 0;

	virtual void on_event( const SDL_Event &event ) = 0;

protected:
	Engine *m_app;
	RenderingSystem *m_rend;
};

} // namespace vv