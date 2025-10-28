#include "vv.hpp"
#include "main_menu.hpp"
#include <iostream>

vv::EngineParameters load_params()
{
	vv::EngineParameters params;

	params.window_height = 1080;
	params.window_width = 1920;
	params.window_title = "ECO+ Deathmatch";

	return params;
}

int main() {
	
	auto params = load_params();

	vv::Engine application (params);

	application.init_systems();

	application.add_layer<MainMenu>();

	application.run();

	application.shutdown_systems();
	
	return 0;
}