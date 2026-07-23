#include "M_transform.h"

transform transform_create() 
{
    transform t = {};
    t.position = vect3_create(0.0f, 0.0f, 0.0f);
    t.rotation = vect3_create(0.0f, 0.0f, 0.0f);
    t.scale = vect3_create(1.0f, 1.0f, 1.0f);
    t.local = mat4_id();
    t.is_dirty = TRUE;
    t.parent = 0;
    return t;
}

transform transform_from_position(vect3 position)
{
    transform t = transform_create();
    t.position = position;
    return t;
}

void transform_position_set(transform* t, vect3 position) 
{
    if (t)
    {
        t->position = position;
        t->is_dirty = TRUE;
    }
}

void transform_rotation_set(transform* t, vect3 rotation)
{
    if (t) 
    {
        t->rotation = rotation;
        t->is_dirty = TRUE;
    }
}

void transform_scale_set(transform* t, vect3 scale)
{
    if (t) 
    {
        t->scale = scale;
        t->is_dirty = TRUE;
    }
}

mat4 transform_get_local(transform* t)
{
    if (t)
    {
        if (t->is_dirty) 
        {
            mat4 tr = mat4_translation(t->position);
            mat4 rot = mat4_eulero_xyz(t->rotation.x, t->rotation.y, t->rotation.z);
            mat4 sc = mat4_scale(t->scale);
            t->local = mat4_mult(tr, mat4_mult(rot, sc));
            t->is_dirty = FALSE;
        }
        return t->local;
    }
    return mat4_id();
}

mat4 transform_get_world(transform* t) 
{
    if (!t) return mat4_id();
    mat4 local = transform_get_local(t);
    if (t->parent) 
    {
        mat4 parent_world = transform_get_world(t->parent);
        return mat4_mult(parent_world, local);
    }

    return local;
}