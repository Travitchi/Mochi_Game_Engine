#include "game.h"
#include <logger.h>
#include "renderer_frontend.h"
#include "M_math.h"
#include "input.h"
#include <SDL_scancode.h>
#include "event.h"

b8 game_initialize(game* game_inst) 
{
    MDEBUG("game_initialize called!");
    game_state* state = (game_state*)game_inst->state;
    game_inst->player_position = vect3_create(0.0f, 0.75f, 6.0f);
    camera* active_cam = camera_system_get_default();
    camera_position_set(active_cam, vect3{ 0.0f, 10.0f, 0.0f });
    camera_rotation_set(active_cam, vect3_create(deg_to_rad(-30.0f), 0.0f, 0.0f));
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
    if (input_is_key_pressed(SDL_SCANCODE_W)) 
    {
        game_inst->player_position.z -= move_speed; 
    }
    if (input_is_key_pressed(SDL_SCANCODE_S)) 
    {
        game_inst->player_position.z += move_speed;
    }
    if (input_is_key_pressed(SDL_SCANCODE_A)) 
    {
        game_inst->player_position.x -= move_speed;
    }
    if (input_is_key_pressed(SDL_SCANCODE_D)) 
    {
        game_inst->player_position.x += move_speed; 
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
    
    /*
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