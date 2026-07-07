#pragma once
#include "defines.hpp"

typedef enum render_buffer_type
{
	RENDER_BUFFER_TYPE_VERTEX,
	RENDER_BUFFER_TYPE_INDEX,
	RENDER_BUFFER_TYPE_UNIFORM
} render_buffer_type;

typedef struct render_buffer
{
	u32 buffer_id;
	u64 total_size;
	u64 stride; //size of a single elemnent
	render_buffer_type type;
} render_buffer;


b8 render_buffer_create(render_buffer* buffer, render_buffer_type type, u64 total_size, u64 stride);
void render_buffer_destroy(render_buffer* buffer);

//tells the gpu the specific buffer to use
void render_buffer_bind(render_buffer* buffer);
void render_buffer_unbind(render_buffer* buffer);

//load and modify the data in the buffer
void render_buffer_load_data(render_buffer* buffer, u64 offset, u64 size, const void* data);
void render_buffer_resize(render_buffer* buffer, u64 new_size);
void render_buffer_copy_to(render_buffer* src, render_buffer* dest, u64 size, u64 source_offset, u64 dest_offset);
