#pragma once
#include <defines.hpp>
#include <game_types.h>
#include "M_math.h"
#include "camera_system.h"

typedef struct game_state
{
	f32 delta_time;
	vect3 player_position;
}game_state;

b8 game_initialize(game* game_inst);

b8 game_update (game* game_inst, f32 delta_time);

b8 game_render (game* game_inst, f32 delta_time);

void game_on_resize(game* game_inst, u32 width, u32 height);
