#include "light_system.h"
#include "M_memory.h"
#include "logger.h"

typedef struct light_system_state
{
    directional_light dir_light;
    point_light p_lights[MAX_POINT_LIGHTS];
    u32 p_light_count;
    vect4 ambient_color;
} light_system_state;

static light_system_state* state_ptr = 0;

b8 light_system_initialize() 
{
    state_ptr = (light_system_state*)Mallocate(sizeof(light_system_state), MEMORY_TAG_RENDERER);
    state_ptr->p_light_count = 0;
    //ambient
    state_ptr->ambient_color = vect4_create(0.0f, 0.0f, 0.0f, 1.0f);
    //sunlight
    state_ptr->dir_light.color = vect4_create(0.8f, 0.8f, 0.8f, 1.0f);
    state_ptr->dir_light.direction = vect4_create(0.57735f, -0.57735f, -0.57735f, 0.0f);

    return TRUE;
}

void light_system_shutdown() 
{
    if (state_ptr) 
    {
        Mfree(state_ptr, sizeof(light_system_state), MEMORY_TAG_RENDERER);
        state_ptr = 0;
    }
}

void light_system_set_directional(vect4 color, vect3 direction)
{
    if (state_ptr) 
    {
        state_ptr->dir_light.color = color;
        state_ptr->dir_light.direction = vect4_create(direction.x, direction.y, direction.z, 0.0f);
    }
}

b8 light_system_add_point_light(vect4 color, vect3 position, f32 constant_f, f32 linear, f32 quadratic) 
{
    if (!state_ptr || state_ptr->p_light_count >= MAX_POINT_LIGHTS)
    {
        MERROR("Max point lights reached or light system not initialized!");
        return FALSE;
    }

    u32 index = state_ptr->p_light_count++;
    state_ptr->p_lights[index].color = color;
    state_ptr->p_lights[index].position = vect4_create(position.x, position.y, position.z, 1.0f);
    state_ptr->p_lights[index].attenuation = vect4_create(constant_f, linear, quadratic, 0.0f);

    return TRUE;
}

void light_system_remove_all_point_lights() 
{
    if (state_ptr) state_ptr->p_light_count = 0;
}

directional_light* light_system_get_directional() 
{
    return state_ptr ? &state_ptr->dir_light : 0;
}

point_light* light_system_get_point_lights()
{
    return state_ptr ? state_ptr->p_lights : 0;
}

u32 light_system_get_point_light_count() 
{
    return state_ptr ? state_ptr->p_light_count : 0;
}

void light_system_set_ambient(vect4 color)
{
    if (state_ptr)
    {
        state_ptr->ambient_color = color;
    }
}

vect4 light_system_get_ambient()
{
    return state_ptr ? state_ptr->ambient_color : vect4_create(0.0f, 0.0f, 0.0f, 1.0f);
}