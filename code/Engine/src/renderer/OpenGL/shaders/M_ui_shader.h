#pragma once
#include "defines.hpp"
#include "M_math.h"

struct render_buffer;

typedef struct M_ui_shader 
{
    u32 shader_id;
    u32 model_location;
    M_ui_shader();
    ~M_ui_shader();

    void M_ui_shader_init();
    void M_ui_shader_use();
    void update_global_state(struct render_buffer* global_ubo, mat4 projection, mat4 view);
    void update_object_state(mat4 model);
} M_ui_shader;