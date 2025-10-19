#include "vv.hpp"

class MainMenu: public vv::Layer
{
public:
	~MainMenu() override;

	void update( double dt_sec ) override;
	void render( double dt_sec ) override;
	void on_event( const SDL_Event &event ) override;
};