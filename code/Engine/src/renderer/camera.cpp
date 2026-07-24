#include "camera.h"
#include "logger.h"

void camera_create(camera* out_camera) 
{
    if (!out_camera) return;
    camera_reset(out_camera);
}

void camera_reset(camera* c) 
{
    if (!c) return;
    c->position = vect3_create(0.0f, 0.0f, 0.0f);
    c->euler_rotation = vect3_create(0.0f, 0.0f, 0.0f);
    c->is_dirty = FALSE;
    c->view_matrix = mat4_id();
    c->fov = deg_to_rad(30.0f);
}

f32 camera_fov_get(const camera* c) 
{
    return c ? c->fov : deg_to_rad(60.0f);
}

void camera_zoom_add(camera* c, f32 amount) 
{
    if (c) 
    {
        c->fov += amount;
        f32 min_fov = deg_to_rad(15.0f);
        f32 max_fov = deg_to_rad(45.0f);
        if (c->fov < min_fov) c->fov = min_fov;
        if (c->fov > max_fov) c->fov = max_fov;
    }
}

vect3 camera_position_get(const camera* c) 
{
    if (c) return c->position;
    return vect3_create(0.0f, 0.0f, 0.0f);
}

void camera_position_set(camera* c, vect3 position) 
{
    if (c) 
    {
        c->position = position;
        c->is_dirty = TRUE;
    }
}

vect3 camera_rotation_get(const camera* c)
{
    if (c) return c->euler_rotation;
    return vect3_create(0.0f, 0.0f, 0.0f);
}

void camera_rotation_set(camera* c, vect3 rotation) 
{
    if (c) 
    {
        c->euler_rotation = rotation;
        c->is_dirty = TRUE;
    }
}

mat4 camera_view_get(camera* c) 
{
    if (c && c->is_dirty)
    {
        mat4 inv_rotation = mat4_eulero_xyz(-c->euler_rotation.x, -c->euler_rotation.y, -c->euler_rotation.z);
        mat4 inv_translation = mat4_translation(vect3_create(-c->position.x, -c->position.y, -c->position.z));
        c->view_matrix = mat4_mult(inv_translation, inv_rotation);
        c->is_dirty = FALSE;
    }
    return c ? c->view_matrix : mat4_id();
}

void camera_move_forward_planar(camera* c, f32 amount) 
{
    if (c) 
    {
        mat4 rotation = mat4_eulero_xyz(c->euler_rotation.x, c->euler_rotation.y, c->euler_rotation.z);
        vect3 forward = mat4_forward(rotation);

        forward.y = 0.0f;
        vect3_normalize(&forward);

        forward = vect3_mult_scale(forward, amount);
        c->position = vect3_add(c->position, forward);
        c->is_dirty = TRUE;
    }
}

void camera_move_backward_planar(camera* c, f32 amount)
{
    camera_move_forward_planar(c, -amount);
}

void camera_move_left(camera* c, f32 amount)
{
    camera_move_right(c, -amount);
}

void camera_move_right(camera* c, f32 amount) 
{
    if (c) 
    {
        mat4 rotation = mat4_eulero_xyz(c->euler_rotation.x, c->euler_rotation.y, c->euler_rotation.z);
        vect3 right = mat4_right(rotation);
        right = vect3_mult_scale(right, amount);
        c->position = vect3_add(c->position, right);
        c->is_dirty = TRUE;
    }
}

void camera_move_up(camera* c, f32 amount)
{
    if (c)
    {
        c->position.y += amount;
        c->is_dirty = TRUE;
    }
}

void camera_move_down(camera* c, f32 amount) 
{
    camera_move_up(c, -amount);
}

void camera_yaw(camera* c, f32 amount) 
{
    if (c)
    {
        c->euler_rotation.y += amount;
        c->is_dirty = TRUE;
    }
}

void camera_pitch(camera* c, f32 amount)
{
    if (c) 
    {
        c->euler_rotation.x += amount;
        if (c->euler_rotation.x > 1.57f) c->euler_rotation.x = 1.57f;
        if (c->euler_rotation.x < -1.57f) c->euler_rotation.x = -1.57f;
        c->is_dirty = TRUE;
    }
}

void camera_follow_target(camera* c, vect3 target_position, vect3 offset, f32 damping, f32 delta_time)
{
    if (c) 
    {
        vect3 desired_position = vect3_add(target_position, offset);
        f32 t = damping * delta_time;
        if (t > 1.0f) t = 1.0f;
        c->position.x += (desired_position.x - c->position.x) * t;
        c->position.y += (desired_position.y - c->position.y) * t;
        c->position.z += (desired_position.z - c->position.z) * t;
        c->is_dirty = TRUE;
    }
}