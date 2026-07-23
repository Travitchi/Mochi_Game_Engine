#include "camera_system.h"
#include "M_memory.h"
#include "logger.h"
#include "hashtable.h"
#include <string.h>

typedef struct camera_lookup 
{
    u16 id;
    u16 reference_count;
    camera c;
} camera_lookup;

typedef struct camera_system_state
{
    camera_system_config config;
    hashtable lookup_table;
    camera_lookup* cameras;
    camera default_camera;
} camera_system_state;

static camera_system_state* state_ptr = 0;

b8 camera_system_initialize(u64* memory_requirement, void* state, camera_system_config config) 
{
    if (config.max_camera_count == 0) return FALSE;
    u64 struct_requirement = sizeof(camera_system_state);
    u64 array_requirement = sizeof(camera_lookup) * config.max_camera_count;
    u64 hashtable_requirement = sizeof(u16) * config.max_camera_count;
    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;
    if (!state) return TRUE;
    state_ptr = (camera_system_state*)state;
    state_ptr->config = config;
    void* array_block = (u8*)state + struct_requirement;
    state_ptr->cameras = (camera_lookup*)array_block;
    void* hashtable_block = (u8*)array_block + array_requirement;
    hashtable_create(sizeof(u16), config.max_camera_count, hashtable_block, FALSE, &state_ptr->lookup_table);

    u16 invalid_id = 0xFFFF;
    hashtable_fill(&state_ptr->lookup_table, &invalid_id);
    for (u16 i = 0; i < config.max_camera_count; ++i) 
    {
        state_ptr->cameras[i].id = invalid_id;
        state_ptr->cameras[i].reference_count = 0;
    }

    camera_create(&state_ptr->default_camera);
    return TRUE;
}

void camera_system_shutdown(void* state) 
{
    if (state_ptr)
    {
        state_ptr = 0;
    }
}

camera* camera_system_acquire(const char* name) 
{
    if (!state_ptr) return 0;

    if (strcmp(name, "default") == 0) 
    {
        return &state_ptr->default_camera;
    }

    u16 id = 0xFFFF;
    if (hashtable_get(&state_ptr->lookup_table, name, &id))
    {
        if (id == 0xFFFF)
        {
            for (u16 i = 0; i < state_ptr->config.max_camera_count; ++i)
            {
                if (state_ptr->cameras[i].id == 0xFFFF)
                {
                    id = i;
                    break;
                }
            }
            if (id == 0xFFFF)
            {
                MERROR("Camera system max capacity reached!");
                return 0;
            }

            state_ptr->cameras[id].id = id;
            camera_create(&state_ptr->cameras[id].c);
            hashtable_set(&state_ptr->lookup_table, name, &id);
        }

        state_ptr->cameras[id].reference_count++;
        return &state_ptr->cameras[id].c;
    }
    return 0;
}

void camera_system_release(const char* name)
{
    if (!state_ptr || strcmp(name, "default") == 0) return;
    u16 id = 0xFFFF;
    if (hashtable_get(&state_ptr->lookup_table, name, &id) && id != 0xFFFF)
    {
        state_ptr->cameras[id].reference_count--;
        if (state_ptr->cameras[id].reference_count < 1)
        {
            state_ptr->cameras[id].id = 0xFFFF;
            hashtable_set(&state_ptr->lookup_table, name, &state_ptr->cameras[id].id);
        }
    }
}

camera* camera_system_get_default() 
{
    if (state_ptr)
    {
        return &state_ptr->default_camera;
    }
    return 0;
}