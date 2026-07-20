#pragma once
#include "defines.hpp"
#include "M_math.h"

#define MAX_POINT_LIGHTS 10

typedef struct directional_light 
{
    vect4 color;
    vect4 direction;
}directional_light;

typedef struct point_light
{
    vect4 color;
    vect4 position;
    vect4 attenuation;
}point_light;

b8 light_system_initialize();
void light_system_shutdown();

void light_system_set_directional(vect4 color, vect3 direction);
b8 light_system_add_point_light(vect4 color, vect3 position, f32 constant_f, f32 linear, f32 quadratic);
void light_system_remove_all_point_lights();

directional_light* light_system_get_directional();
point_light* light_system_get_point_lights();
u32 light_system_get_point_light_count();