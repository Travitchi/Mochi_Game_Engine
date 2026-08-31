#include "game.h"
#include <logger.h>
#include "renderer_frontend.h"
#include "M_math.h"
#include "input.h"
#include <SDL_scancode.h>
#include "event.h"
#include "sprite_manager_system.h"

b8 game_initialize(game* game_inst) 
{
    MDEBUG("game_initialize called!");
    game_state* state = (game_state*)game_inst->state;
    game_inst->player_position = vect3_create(0.0f, 0.75f, 6.0f);
    camera* active_cam = camera_system_get_default();
    camera_position_set(active_cam, vect3{ 0.0f, 10.0f, 0.0f });
    camera_rotation_set(active_cam, vect3_create(deg_to_rad(-30.0f), 0.0f, 0.0f));
    
	// make sure to initialize the walk_south animation
    static sprite_keyframe walk_s[4] = { {6,4}, {5,4}, {6,4}, {7,4} };
    static sprite_keyframe walk_n[4] = { {6,0}, {5,0}, {6,0}, {7,0} };
    static sprite_keyframe walk_e[4] = { {6,6}, {5,6}, {6,6}, {7,6} };
    static sprite_keyframe walk_w[4] = { {6,2}, {5,2}, {6,2}, {7,2} };

    // 2. Setup the South Animation
    state->walk_south.frames = walk_s;
    state->walk_south.frame_count = 4;
    state->walk_south.frame_time = 0.12f;

    // 3. Setup the North Animation
    state->walk_north.frames = walk_n;
    state->walk_north.frame_count = 4;
    state->walk_north.frame_time = 0.12f;

    // 4. Setup the East/West Animation
    state->walk_east.frames = walk_e;
    state->walk_east.frame_count = 4;
    state->walk_east.frame_time = 0.12f;


	state->walk_west.frames = walk_w;
    state->walk_west.frame_count = 4;
    state->walk_west.frame_time = 0.12f;

    state->current_idle.row = 6;
    state->current_idle.col = 4;

    return TRUE;
}

b8 game_update(game* game_inst, f32 delta_time)
{
    game_state* state = (game_state*)game_inst->state;

   camera* active_cam = camera_system_get_default();
    f32 move_speed = 5.0f * delta_time;
    //f32 rot_speed = 1.0f * delta_time;
    /*
    if (input_is_key_pressed(SDL_SCANCODE_LEFT)) { camera_yaw(active_cam, rot_speed); }
    if (input_is_key_pressed(SDL_SCANCODE_RIGHT)) { camera_yaw(active_cam, -rot_speed); }
    if (input_is_key_pressed(SDL_SCANCODE_UP)) { camera_pitch(active_cam, rot_speed); }
    if (input_is_key_pressed(SDL_SCANCODE_DOWN)) { camera_pitch(active_cam, -rot_speed); }
    */

    animated_sprite* active_anim = 0;
    sprite* hero = sprite_system_get_sprite("hero");
    if (input_is_key_pressed(SDL_SCANCODE_W))
    {
        game_inst->player_position.z -= move_speed;
        active_anim = &state->walk_north;
        state->current_idle = { 6, 0 };
    }
    else if (input_is_key_pressed(SDL_SCANCODE_S))
    {
        game_inst->player_position.z += move_speed;
        active_anim = &state->walk_south;
        state->current_idle = { 6, 4 };
    }
    else if (input_is_key_pressed(SDL_SCANCODE_A))
    {
        game_inst->player_position.x -= move_speed;
        active_anim = &state->walk_east;
        state->current_idle = { 6, 6 };
        
    }
    else if (input_is_key_pressed(SDL_SCANCODE_D))
    {
        game_inst->player_position.x += move_speed;
        active_anim = &state->walk_west;
        state->current_idle = { 6, 2 };
        
    }

    if (active_anim)
    {
        sprite_animation_update(hero, active_anim, delta_time);
    }
    else
    {
        sprite_set_frame(hero, state->current_idle.row, state->current_idle.col);
    }

    if (input_is_key_pressed(SDL_SCANCODE_SPACE))
    { 
        camera_move_up(active_cam, move_speed); 
    }
    if (input_is_key_pressed(SDL_SCANCODE_LSHIFT)) 
    {
        camera_move_down(active_cam, move_speed);
    }

    vect3 camera_offset = vect3_create(0.0f, 12.0f, 18.0f);
    vect3 new_cam_pos = vect3_add(game_inst->player_position, camera_offset);
    camera_position_set(active_cam, new_cam_pos);
    
    /* Remove the comment to see the active camera position on the console
    if (active_cam->is_dirty)
    {
        vect3 pos = camera_position_get(active_cam);
        vect3 rot = camera_rotation_get(active_cam);

        MINFO("Cam Pos [ X: %.2f | Y: %.2f | Z: %.2f ] || Rot [ P: %.2f | Y: %.2f ]",
            pos.x, pos.y, pos.z, rot.x, rot.y);
    }
    */
    return TRUE;
}

b8 game_render(game* game_inst, f32 delta_time)
{
    renderer_push_world_matrices();
	return TRUE;
}

void game_on_resize(game* game_inst, u32 width, u32 height)
{

}