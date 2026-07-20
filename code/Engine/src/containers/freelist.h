#pragma once
#include "defines.hpp"

typedef struct freelist 
{
    void* memory;
} freelist;

//Call twice: once with memory=0 to get memory_requirement then again with an allocated block of memory.
KAPI void freelist_create(u64 total_size, u64* memory_requirement, void* memory, freelist* out_list);

KAPI void freelist_destroy(freelist* list);

KAPI b8 freelist_allocate_block(freelist* list, u64 size, u64* out_offset);

KAPI b8 freelist_free_block(freelist* list, u64 size, u64 offset);

KAPI void freelist_clear(freelist* list);

KAPI u64 freelist_free_space(freelist* list);

KAPI b8 freelist_resize(freelist* list, u64* memory_requirement, void* new_memory, u64 new_size, void** out_old_memory);