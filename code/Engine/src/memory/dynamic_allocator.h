#pragma once
#include "defines.hpp"

typedef struct dynamic_allocator 
{
    void* memory;
} dynamic_allocator;

// Called twice once to get the exact memory requirement then again with the allocated block.
KAPI b8 dynamic_allocator_create(u64 total_size, u64* memory_requirement, void* memory, dynamic_allocator* out_allocator);

// Destroys the allocator and its internal free list
KAPI b8 dynamic_allocator_destroy(dynamic_allocator* allocator);

// Hands out a block of usable memory
KAPI void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size);

// Returns a block of memory back to the allocator
KAPI b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size);

// Returns the amount of free space remaining in the allocator
KAPI u64 dynamic_allocator_free_space(dynamic_allocator* allocator);