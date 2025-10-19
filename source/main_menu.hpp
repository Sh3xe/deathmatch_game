#include "vv.hpp"

class MainMenu: public vv::Layer
{
public:
	~MainMenu() override;

	void update() override;
	void render() override;
};