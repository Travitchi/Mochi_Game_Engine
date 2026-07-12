#include "dynamic_allocator.h"
#include "freelist.h"
#include "logger.h"
#include "M_memory.h"

typedef struct dynamic_allocator_state
{
    u64 total_size;
    freelist list;
    void* freelist_block;
    void* memory_block;
} dynamic_allocator_state;


b8 dynamic_allocator_create(u64 total_size, u64* memory_requirement, void* memory, dynamic_allocator* out_allocator)
{
    if (total_size < 1)
    {
        MERROR("dynamic_allocator_create cannot have a total_size of 0.");
        return FALSE;
    }

    u64 freelist_requirement = 0;
    freelist_create(total_size, &freelist_requirement, 0, 0);
    // Allocator State + Free List State + The Actual Usable Memory
    *memory_requirement = sizeof(dynamic_allocator_state) + freelist_requirement + total_size;

    if (!memory)
    {
        return TRUE;
    }

    out_allocator->memory = memory;
    dynamic_allocator_state* state = (dynamic_allocator_state*)out_allocator->memory;
    state->total_size = total_size;
    state->freelist_block = (u8*)memory + sizeof(dynamic_allocator_state);
    state->memory_block = (u8*)state->freelist_block + freelist_requirement;

    freelist_create(total_size, &freelist_requirement, state->freelist_block, &state->list);
    Mzero_memory(state->memory_block, total_size);

    return TRUE;
}

b8 dynamic_allocator_destroy(dynamic_allocator* allocator) 
{
    {
        dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->memory;
        freelist_destroy(&state->list);
        Mzero_memory(state->memory_block, state->total_size);
        allocator->memory = 0;
        return TRUE;
    }
    return FALSE;
}

void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size)
{
    if (!allocator || !allocator->memory || size == 0) return 0;

    dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->memory;
    u64 offset = 0;

    if (freelist_allocate_block(&state->list, size, &offset))
    {
        return (u8*)state->memory_block + offset;
    }

    MERROR("Dynamic Allocator failed to allocate %llu bytes.", size);
    return 0;
}

b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size)
{
    if (!allocator || !allocator->memory || !block || size == 0) return FALSE;

    dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->memory;
    if (block < state->memory_block || block >= ((u8*)state->memory_block + state->total_size))
    {
        MERROR("Dynamic Allocator: Attempted to free a block outside of its memory range!");
        return FALSE;
    }
    u64 offset = (u8*)block - (u8*)state->memory_block;
    return freelist_free_block(&state->list, size, offset);
}

u64 dynamic_allocator_free_space(dynamic_allocator* allocator) 
{
    if (!allocator || !allocator->memory) return 0;

    dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->memory;
    return freelist_free_space(&state->list);
}