#include "render_buffer.h"
#include "glad.h"
#include "logger.h"
#include "M_memory.h"


static u32 get_opengl_target(render_buffer_type type)
{
	switch (type)
	{
	case RENDER_BUFFER_TYPE_VERTEX:  return GL_ARRAY_BUFFER;
	case RENDER_BUFFER_TYPE_INDEX:   return GL_ELEMENT_ARRAY_BUFFER;
	case RENDER_BUFFER_TYPE_UNIFORM: return GL_UNIFORM_BUFFER;
	default:                         return GL_ARRAY_BUFFER;
	}
}


b8 render_buffer_create(render_buffer* buff, render_buffer_type type, u64 total_size, u64 stride)
{

	Mzero_memory(buff, sizeof(render_buffer));
	buff->type = type;
	buff->total_size = total_size;
	buff->stride = stride;

	glGenBuffers(1, &buff->buffer_id);

	if (buff->buffer_id == 0)
	{
		MERROR("Failed to create render buffer");
		return false;
	}

	u32 target = get_opengl_target(buff->type);
	glBindBuffer(target, buff->buffer_id);
	glBufferData(target, total_size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(target, 0);

	return TRUE;
}

void render_buffer_destroy(render_buffer* buff) 
{
	if (buff->buffer_id != 0) 
	{
		glDeleteBuffers(1, &buff->buffer_id);
		Mzero_memory(buff, sizeof(render_buffer));
	}
}

void render_buffer_bind(render_buffer* buff) 
{
	u32 target = get_opengl_target(buff->type);
	glBindBuffer(target, buff->buffer_id);
}

void render_buffer_unbind(render_buffer* buff) 
{
	u32 target = get_opengl_target(buff->type);
	glBindBuffer(target, 0);
}

void render_buffer_load_data(render_buffer* buff, u64 offset, u64 size, const void* data) 
{

	if (!buff || buff->buffer_id == 0 || !data)
	{
		return;
	}

	u32 target = get_opengl_target(buff->type);
	glBindBuffer(target, buff->buffer_id);

	if (offset == 0 && size == buff->total_size)
	{
		glBufferData(target, size, data, GL_STATIC_DRAW);
	}
	else
	{
		glBufferSubData(target, offset, size, data);
	}
}

void render_buffer_resize(render_buffer* buff, u64 new_size) 
{
	if (!buff || buff->buffer_id == 0 || new_size == buff->total_size) 
	{
		return;
	}

	u32 target = get_opengl_target(buff->type);
	glBindBuffer(target, buff->buffer_id);
	glBufferData(target, new_size, nullptr, GL_DYNAMIC_DRAW);
	buff->total_size = new_size;
	glBindBuffer(target, 0);
}

void render_buffer_copy_to(render_buffer* src, render_buffer* dest, u64 size, u64 source_offset, u64 dest_offset) 
{
	if (!src || !dest || src->buffer_id == 0 || dest->buffer_id == 0) 
	{
		return;
	}

	glBindBuffer(GL_COPY_READ_BUFFER, src->buffer_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, dest->buffer_id);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, source_offset, dest_offset, size);

	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}