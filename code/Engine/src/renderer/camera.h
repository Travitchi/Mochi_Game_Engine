#pragma once
#include "defines.hpp"
#include "M_math.h"

typedef struct camera 
{
    vect3 position;
    vect3 euler_rotation;
    b8 is_dirty;
    mat4 view_matrix;
    f32 fov;
} camera;

KAPI void camera_create(camera* out_camera);
KAPI void camera_reset(camera* c);
KAPI f32 camera_fov_get(const camera* c);
KAPI void camera_zoom_add(camera* c, f32 amount);

KAPI vect3 camera_position_get(const camera* c);
KAPI void camera_position_set(camera* c, vect3 position);

KAPI vect3 camera_rotation_get(const camera* c);
KAPI void camera_rotation_set(camera* c, vect3 rotation);

KAPI mat4 camera_view_get(camera* c);

KAPI void camera_move_forward_planar(camera* c, f32 amount);
KAPI void camera_move_backward_planar(camera* c, f32 amount);
KAPI void camera_move_left(camera* c, f32 amount);
KAPI void camera_move_right(camera* c, f32 amount);
KAPI void camera_move_up(camera* c, f32 amount);
KAPI void camera_move_down(camera* c, f32 amount);
KAPI void camera_yaw(camera* c, f32 amount);
KAPI void camera_pitch(camera* c, f32 amount);
KAPI void camera_follow_target(camera* c, vect3 target_position, vect3 offset, f32 damping, f32 delta_time);
