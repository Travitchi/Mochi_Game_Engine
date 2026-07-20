#pragma once

#include "defines.hpp"

typedef enum memory_tag
{
	MEMORY_TAG_UNKNOWN, //default
	MEMORY_TAG_ARRAY,
	MEMORY_TAG_DARRAY, 
	MEMORY_TAG_DICT, 
	MEMORY_TAG_RING_QUEUE,
	MEMORY_TAG_BST,
	MEMORY_TAG_STRING,
	MEMORY_TAG_APPLICATION,
	MEMORY_TAG_JOB,
	MEMORY_TAG_MATERIAL_INSTANCE,
	MEMORY_TAG_RENDERER,
	MEMORY_TAG_GAME,
	MEMORY_TAG_TRANSFORM,
	MEMORY_TAG_ENTITY,
	MEMORY_TAG_ENTITY_NODE,
	MEMORY_TAG_SCENE,

	MEMORY_TAG_MAX_TAGS
} memory_tag;

KAPI void initialize_memory();
KAPI void shutdown_memory();

KAPI void* Mallocate(u64 size, memory_tag tag); //similar to malloc, but with a tag for tracking purposes
KAPI void Mfree(void* block, u64 size, memory_tag tag);
KAPI void* Mzero_memory(void* block, u64 size); 
KAPI void* Mcopy_memory(void* dest, const void* src, u64 size);
KAPI void* Mset_memory(void* dest, i32 value, u64 size);

KAPI char* get_memory_usage_str();
