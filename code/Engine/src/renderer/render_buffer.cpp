#include "render_buffer.h"
#include "glad.h"
#include "logger.h"
#include "M_memory.h"
#include "freelist.h"


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

	u64 requirement = 0;
	freelist_create(total_size, &requirement, 0, 0);
	buff->freelist_memory_requirement = requirement;
	buff->freelist_block = Mallocate(requirement, MEMORY_TAG_RENDERER);

	freelist_create(total_size, &requirement, buff->freelist_block, &buff->buffer_freelist);

	return TRUE;
}

void render_buffer_destroy(render_buffer* buff) 
{
	if (buff->buffer_id != 0) 
	{
		glDeleteBuffers(1, &buff->buffer_id);
	}

	if (buff->freelist_block) 
	{
		freelist_destroy(&buff->buffer_freelist);
		
	}
	Mfree(buff->freelist_block, buff->freelist_memory_requirement, MEMORY_TAG_RENDERER);
	Mzero_memory(buff, sizeof(render_buffer));
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
	if (!buff || buff->buffer_id == 0 || new_size <= buff->total_size) return;
	u32 target = get_opengl_target(buff->type);
	u32 new_buffer_id;
	glGenBuffers(1, &new_buffer_id);
	glBindBuffer(target, new_buffer_id);
	glBufferData(target, new_size, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_COPY_READ_BUFFER, buff->buffer_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, new_buffer_id);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, buff->total_size);

	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glBindBuffer(target, 0);

	glDeleteBuffers(1, &buff->buffer_id);
	buff->buffer_id = new_buffer_id;
	buff->total_size = new_size;

	u64 new_requirement = 0;
	freelist_resize(&buff->buffer_freelist, &new_requirement, 0, new_size, 0);
	void* new_freelist_block = Mallocate(new_requirement, MEMORY_TAG_RENDERER);
	void* old_freelist_block = 0;
	if (freelist_resize(&buff->buffer_freelist, &new_requirement, new_freelist_block, new_size, &old_freelist_block))
	{
		Mfree(old_freelist_block, buff->freelist_memory_requirement, MEMORY_TAG_RENDERER);
		buff->freelist_block = new_freelist_block;
		buff->freelist_memory_requirement = new_requirement;
	}
	else 
	{
		MERROR("Failed to resize freelist for render buffer!");
		Mfree(new_freelist_block, new_requirement, MEMORY_TAG_RENDERER);
	}
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

b8 render_buffer_allocate(render_buffer* buffer, u64 size, u64* out_offset)
{
	if (!buffer || !out_offset) return FALSE;
	return freelist_allocate_block(&buffer->buffer_freelist, size, out_offset);
}

b8 render_buffer_free(render_buffer* buffer, u64 size, u64 offset)
{
	if (!buffer) return FALSE;
	return freelist_free_block(&buffer->buffer_freelist, size, offset);
}