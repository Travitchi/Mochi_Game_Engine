#pragma once
#include"application.h"
#include "game_types.h"
#include "logger.h"
#include "M_memory.h"

extern b8 create_game(game* game_out);
//actual main entry point of the application
int main(void) {

	initialize_memory();

	game game_inst;
	if (!create_game(&game_inst))
	{
		MFATAL("Could not create game!");
		return -1;
	}

	//make sure that function pointers exist
	if(!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize)
	{
		MFATAL("The game's function pointers must be assigned");
		return -2;
	}

	//initialization
	if (!application_create(&game_inst))
	{
		MINFO("Application failed to create!");
		return 1;
	}

	//game loop beginning
	if (!application_run())
	{
		MINFO("Application did not shutdown normally!");
		return 2;
	}

	shutdown_memory();
	return 0;
}