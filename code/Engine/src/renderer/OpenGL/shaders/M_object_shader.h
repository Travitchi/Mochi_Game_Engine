#pragma once
#include "defines.hpp"
#include "M_math.h"

struct render_buffer;

typedef struct M_obj_shader
{
	u32 shader_id;
	u32 model_location;
	u32 normal_location;

	M_obj_shader();
	~M_obj_shader();

	void M_obj_shader_init();

	//core pipeline functions
	void M_obj_shader_use();

	void update_glob_state(struct render_buffer* global_ubo, mat4 projection, mat4 view);
	void update_object_state(mat4 model, mat4 normal_matrix);
	

}M_obj_shader;