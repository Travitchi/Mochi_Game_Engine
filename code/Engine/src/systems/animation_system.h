#pragma once
#include "defines.hpp"
#include "game_types.h"
#include "sprite_manager_system.h"

struct sprite_keyframe
{
	
	u32 row;
	u32 col;
};

struct animated_sprite
{
	sprite_keyframe* frames; // array of frames
	u32 frame_count;  //total frames
	u32 current_index; // which frame is currently displayed
	f32 frame_time; //animation speed
	f32 timer; //track delta time
};

KAPI void sprite_animation_update(sprite* s, animated_sprite* anim, f32 delta_time);