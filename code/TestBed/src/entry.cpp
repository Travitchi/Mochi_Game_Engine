// This is aka main.cpp we're calling it entry just for technical word placement and accuracy

#include <entry.h>
#include "game.h"
//remove platform.h later
#include <M_memory.h>

b8 create_game(game* game_out) {
	//application's configuration

	game_out->app_config.start_pos_x = 100;
	game_out->app_config.start_pos_y = 100;
	game_out->app_config.start_width = 1280;
	game_out->app_config.start_height = 720;
	game_out->app_config.name = (char*)"Mochi Game Engine";

	game_out->update = game_update;
	game_out->render = game_render;
	game_out->initialize = game_initialize;
	game_out->on_resize = game_on_resize;

	game_out->state_memory_requirement = sizeof(game_state);
	game_out->state = 0;

	return TRUE;
}

