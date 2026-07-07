#pragma once
#include <defines.hpp>
#include <game_types.h>
#include "M_math.h"
typedef struct game_state
{
	f32 delta_time;
	mat4 view;
	vect3 camera_position;
	vect3 camera_euler;
}game_state;

b8 game_initialize(game* game_inst);

b8 game_update (game* game_inst, f32 delta_time);

b8 game_render (game* game_inst, f32 delta_time);

void game_on_resize(game* game_inst, u32 width, u32 height);


//test hack
void camera_recalculate_view_matrix(game_state* state);
void camera_yaw(game_state* state, f32 amount);
void camera_pitch(game_state* state, f32 amount);

// Camera Translation (Walking/Flying)
void camera_move_forward(game_state* state, f32 amount);
void camera_move_right(game_state* state, f32 amount);
void camera_move_up(game_state* state, f32 amount);