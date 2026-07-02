#include "M_object_shader.h"
#include <glad.h>
#include "M_shader_utils.h"
#include "logger.h"

//constructor and destructor
M_obj_shader::M_obj_shader() {
	shader_id = 0;
	projection_location = 0;
	view_location = 0;
	model_location = 0;
}

void M_obj_shader::M_obj_shader_init(){
    shader_id = create_shader_program("Assets/M_object_shader.vert", "Assets/M_object_shader.frag");

    if (shader_id == 0) {
        MERROR("Failed to build the Object Shader program.");
        return;
    }

    MINFO("Object Shader program successfully built and loaded.");

    projection_location = glGetUniformLocation(shader_id, "projection");
    view_location = glGetUniformLocation(shader_id, "view");
    model_location = glGetUniformLocation(shader_id, "model");
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
void M_obj_shader::update_glob_state(mat4 projection, mat4 view)
{
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data);
}

//sends the model matrix to the shader to update the object state
void M_obj_shader::update_object_state(mat4 model)
{
	glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data);
}

