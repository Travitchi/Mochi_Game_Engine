#include "M_object_shader.h"
#include <glad.h>
#include "M_shader_utils.h"
#include "logger.h"
#include "render_buffer.h"



//constructor and destructor
M_obj_shader::M_obj_shader() {
	shader_id = 0;
    model_location = 0;
    normal_location = 0;
}

void M_obj_shader::M_obj_shader_init(){
    shader_id = create_shader_program("M_object_shader.vert", "M_object_shader.frag");

    if (shader_id == 0) {
        MERROR("Failed to build the Object Shader program.");
        return;
    }

    MINFO("Object Shader program successfully built and loaded.");

    
    model_location = glGetUniformLocation(shader_id, "u_push.model");
    normal_location = glGetUniformLocation(shader_id, "u_push.normal_matrix");
}



M_obj_shader::~M_obj_shader()
{
	glDeleteProgram(shader_id);
	MDEBUG("Object Shader program destroyed.");
}

//tells the GPU to use this shader
void M_obj_shader::M_obj_shader_use()
{
	glUseProgram(shader_id);
}

//updates the shader on the obj in case we move with our camera
void M_obj_shader::update_glob_state(render_buffer* global_ubo, mat4 projection, mat4 view)
{
    render_buffer_load_data(global_ubo, 0, sizeof(mat4), projection.data);
    render_buffer_load_data(global_ubo, sizeof(mat4), sizeof(mat4), view.data);
}

// sends the model matrix to the shader to update the object state
void M_obj_shader::update_object_state(mat4 model, mat4 normal_matrix)
{
    glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data);

    glUniformMatrix4fv(normal_location, 1, GL_FALSE, normal_matrix.data);
}


