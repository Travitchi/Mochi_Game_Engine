#include "animation_system.h"


void sprite_animation_update(sprite* s, animated_sprite* anim, f32 delta_time)
{
	if (!s || !anim || anim->frame_count == 0) 
		return;
	anim->timer += delta_time;
	
	while (anim->timer >= anim->frame_time)
	{
		anim->timer -= anim->frame_time;
		anim->current_index++;
		if (anim->current_index >= anim->frame_count)
		{
			anim->current_index = 0;
		}
		sprite_keyframe current_frame = anim->frames[anim->current_index];
		sprite_set_frame(s, current_frame.row, current_frame.col);
	}

}