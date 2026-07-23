#pragma once
#include "defines.hpp"
#include "camera.h"

typedef struct camera_system_config
{
    u16 max_camera_count;
} camera_system_config;

KAPI b8 camera_system_initialize(u64* memory_requirement, void* state, camera_system_config config);
KAPI void camera_system_shutdown(void* state);
KAPI camera* camera_system_acquire(const char* name);
KAPI void camera_system_release(const char* name);
KAPI camera* camera_system_get_default();