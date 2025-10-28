#include "vv.hpp"

class MainMenu: public vv::Layer
{
public:
	~MainMenu() override;

	vv::Error init() override;

	void shutdown() override;

	void update( double dt_sec ) override;

	void render( double dt_sec ) override;

	void on_event( const SDL_Event &event ) override;

private:
	vv::Scene2D m_scene;
	float m_time;
};