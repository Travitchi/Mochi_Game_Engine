#pragma once
#include "defines.hpp"
#include "M_math.h" 

KAPI transform transform_create();
KAPI transform transform_from_position(vect3 position);

KAPI void transform_position_set(transform* t, vect3 position);
KAPI void transform_rotation_set(transform* t, vect3 rotation);
KAPI void transform_scale_set(transform* t, vect3 scale);

KAPI mat4 transform_get_local(transform* t);
KAPI mat4 transform_get_world(transform* t);