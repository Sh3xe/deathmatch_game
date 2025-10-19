#include "application.hpp"
#include "main_menu.hpp"
#include <iostream>

vv::ApplicationParameters load_params()
{
	vv::ApplicationParameters params;

	params.window_height = 1080;
	params.window_width = 1920;
	params.window_title = "ECO+ Deathmatch";

	return params;
}

int main() {
	
	auto params = load_params();

	vv::Application application (params);

	application.init_modules();

	application.add_layer<MainMenu>();

	application.run();
	
	return 0;
}