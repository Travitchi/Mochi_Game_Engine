#include "M_ui_shader.h"
#include <glad.h>
#include "M_shader_utils.h"
#include "logger.h"
#include "render_buffer.h"

M_ui_shader::M_ui_shader() 
{ 
    shader_id = 0;
    model_location = 0;
}
M_ui_shader::~M_ui_shader() 
{
    glDeleteProgram(shader_id); 
}

void M_ui_shader::M_ui_shader_init()
{
    shader_id = create_shader_program("M_ui_shader.vert", "M_ui_shader.frag");
    if (shader_id == 0) 
    {
        MERROR("Failed to build UI Shader.");
        return; 
    }
    model_location = glGetUniformLocation(shader_id, "u_push.model");
}

void M_ui_shader::M_ui_shader_use()
{ 
    glUseProgram(shader_id);
}

void M_ui_shader::update_global_state(render_buffer* global_ubo, mat4 projection, mat4 view) 
{
    render_buffer_load_data(global_ubo, 0, sizeof(mat4), projection.data);
    render_buffer_load_data(global_ubo, sizeof(mat4), sizeof(mat4), view.data);
}

void M_ui_shader::update_object_state(mat4 model) 
{
    glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data);
}