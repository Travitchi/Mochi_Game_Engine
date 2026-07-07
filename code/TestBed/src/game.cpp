#include "game.h"
#include <logger.h>
#include "renderer_frontend.h"
#include "M_math.h"
#include "input.h"
#include <SDL_scancode.h>
#include "event.h"

//hack test
void camera_recalculate_view_matrix(game_state* state)
{
	mat4 rotation = mat4_eulero_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
	mat4 translation = mat4_translation(state->camera_position);

	// FIX 1: Changed order to Translation * Rotation (First-Person local rotation)
	mat4 world_transform = mat4_mult(translation, rotation);
	state->view = mat4_inverse(world_transform);

	renderer_set_view(state->view);
}

void camera_yaw(game_state* state, f32 amount)
{
	state->camera_euler.y += amount;
	f32 full_circle = 2.0f * M_PI;
	while (state->camera_euler.y > full_circle) { state->camera_euler.y -= full_circle; }
	while (state->camera_euler.y < -full_circle) { state->camera_euler.y += full_circle; }

	camera_recalculate_view_matrix(state);
}

void camera_pitch(game_state* state, f32 amount)
{
	state->camera_euler.x += amount;
	f32 limit = deg_to_rad(89.0f);
	if (state->camera_euler.x > limit) { state->camera_euler.x = limit; }
	if (state->camera_euler.x < -limit) { state->camera_euler.x = -limit; }

	camera_recalculate_view_matrix(state);
}

// =========================================================================
// NEW: CAMERA TRANSLATION (WALKING / FLYING)
// =========================================================================
void camera_move_forward(game_state* state, f32 amount) {
	mat4 rotation = mat4_eulero_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
	vect3 forward = mat4_forward(rotation);
	forward = vect3_mult_scale(forward, amount);
	state->camera_position = vect3_add(state->camera_position, forward);
	camera_recalculate_view_matrix(state);
}

void camera_move_right(game_state* state, f32 amount) {
	mat4 rotation = mat4_eulero_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
	vect3 right = mat4_right(rotation);
	right = vect3_mult_scale(right, amount);
	state->camera_position = vect3_add(state->camera_position, right);
	camera_recalculate_view_matrix(state);
}

void camera_move_up(game_state* state, f32 amount) {
	state->camera_position.y += amount;
	camera_recalculate_view_matrix(state);
}

b8 game_initialize(game* game_inst)
{
	//remove when not debugging
	MDEBUG("game_initialize called!");
	//test hack
	game_state* state = (game_state*)game_inst->state;
	state->camera_position = vect3{ 0.0f, 0.0f, 3.0f };
	state->camera_euler = vect3{ 0.0f, 0.0f, 0.0f };
	camera_recalculate_view_matrix(state);
	return TRUE;
}

b8 game_update(game* game_inst, f32 delta_time)
{
	//test hack
	
	game_state* state = (game_state*)game_inst->state;
	f32 move_speed = 5.0f * delta_time;
	f32 rotation_speed = 2.0f * delta_time;

	// LOOKING (Arrow Keys)
	if (input_is_key_pressed(SDL_SCANCODE_LEFT)) { camera_yaw(state, rotation_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_RIGHT)) { camera_yaw(state, -rotation_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_UP)) { camera_pitch(state, rotation_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_DOWN)) { camera_pitch(state, -rotation_speed); }

	// WALKING (W A S D)
	if (input_is_key_pressed(SDL_SCANCODE_W)) { camera_move_forward(state, move_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_S)) { camera_move_forward(state, -move_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_A)) { camera_move_right(state, -move_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_D)) { camera_move_right(state, move_speed); }

	// FLYING UP/DOWN (E / Q)
	if (input_is_key_pressed(SDL_SCANCODE_E)) { camera_move_up(state, move_speed); }
	if (input_is_key_pressed(SDL_SCANCODE_Q)) { camera_move_up(state, -move_speed); }

	if (input_is_key_pressed(SDL_SCANCODE_T))
	{
		MDEBUG("Swapping texture!");
		event_context context = {};
		event_fire(0x10, game_inst, context);
	}
	//
	return TRUE;
}

b8 game_render(game* game_inst, f32 delta_time)
{
	return TRUE;
}

void game_on_resize(game* game_inst, u32 width, u32 height)
{

}