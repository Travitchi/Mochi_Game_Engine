#pragma once
#include "defines.hpp"

typedef enum buttons
{
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_MIDDLE,
	BUTTON_MAX_BUTTONS
}buttons;

#define DEFINE_KEY(name, code) KEY_##name = code

typedef u16 keys;

void input_initialize();
void input_shutdown();
void input_update(f64 delta_time);

//keyboard input
KAPI b8 input_is_key_pressed(keys);
KAPI b8 input_is_key_released(keys);
KAPI b8 input_was_key_pressed(keys);
KAPI b8 input_was_key_released(keys);

void input_process_key(keys key, b8 pressed);

//mouse input
KAPI b8 input_is_button_pressed(buttons button);
KAPI b8 input_is_button_released(buttons button);
KAPI b8 input_was_button_pressed(buttons button);
KAPI b8 input_was_button_released(buttons button);
KAPI void input_get_mouse_position(i32* x, i32* y);
KAPI void input_getmouse_previous_position(i32* x, i32* y);

void input_process_button(buttons button, b8 pressed);
void input_process_mouse_move(i16 x, i16 y);
void input_process_mouse_wheel(i8 z_delta);