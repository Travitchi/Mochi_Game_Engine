#pragma once
#include "defines.hpp"
#include "M_math.h"


typedef struct M_obj_shader
{
	u32 shader_id;

	u32 projection_location;
	u32 view_location;
	u32 model_location;

	M_obj_shader();
	~M_obj_shader();

	void M_obj_shader_init();

	//core pipeline functions
	void M_obj_shader_use();

	//updates the shader on the obj in case we move with our camera
	void update_glob_state(mat4 projection, mat4 view);

	void update_object_state(mat4 model);

}M_obj_shader;