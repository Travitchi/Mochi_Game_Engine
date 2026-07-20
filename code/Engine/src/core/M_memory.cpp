#include "M_memory.h"
#include "logger.h"
#include "Platform.h"
#include "dynamic_allocator.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static const char* memory_tag_string[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN",
    "ARRAY",
    "DARRAY",
    "DICT",
    "RING_QUEUE",
    "BST",
    "STRING",
    "APPLICATION",
    "JOB",
    "MAT_INST",
    "RENDERER",
    "GAME",
    "TRANSFORM",
    "ENTITY",
    "ENTITY_NODE",
    "SCENE"
};

typedef struct memory_system_configuration 
{
    u64 total_alloc_size;
} memory_system_configuration;

typedef struct memory_system_state
{
    memory_system_configuration config;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
    u64 alloc_count;
    u64 allocator_memory_requirement;
    dynamic_allocator allocator;
    void* allocator_block;
} memory_system_state;

static memory_system_state* state_ptr;

void initialize_memory() 
{
    memory_system_configuration config = {};
    config.total_alloc_size = GIBIBYTES(1);
    u64 state_memory_requirement = sizeof(memory_system_state);
    u64 alloc_requirement = 0;
    dynamic_allocator_create(config.total_alloc_size, &alloc_requirement, 0, 0);

    void* block = platform_allocate(state_memory_requirement + alloc_requirement, FALSE);
    if (!block) 
    {
        MFATAL("Memory system failed to allocate master block!");
        return;
    }

    state_ptr = (memory_system_state*)block;
    state_ptr->config = config;
    state_ptr->alloc_count = 0;
    state_ptr->allocator_memory_requirement = alloc_requirement;

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) 
    {
        state_ptr->tagged_allocations[i] = 0;
    }

    state_ptr->allocator_block = ((u8*)block + state_memory_requirement);

    if (!dynamic_allocator_create(state_ptr->config.total_alloc_size, &state_ptr->allocator_memory_requirement, state_ptr->allocator_block, &state_ptr->allocator))
    {
        MFATAL("Memory system failed to create internal dynamic allocator!");
        return;
    }

    MINFO("Memory system fully initialized with 1 GiB of custom managed memory.");
}

void shutdown_memory()
{
    if (state_ptr) 
    {
        dynamic_allocator_destroy(&state_ptr->allocator);
        platform_free(state_ptr, FALSE);
        state_ptr = 0;
    }
}

void* Mallocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) 
    {
        MWARN("Mallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if (state_ptr)
    {
        state_ptr->alloc_count++;
        state_ptr->tagged_allocations[tag] += size;

        void* block = dynamic_allocator_allocate(&state_ptr->allocator, size);
        if (block) 
        {
            Mzero_memory(block, size);
            return block;
        }
        else
        {
            MFATAL("Dynamic Allocator failed to allocate %llu bytes!", size);
            return 0;
        }
    }

    MERROR("Mallocate called before initialize_memory! Memory will not be allocated.");
    return 0;
}

void Mfree(void* block, u64 size, memory_tag tag) 
{
    if (tag == MEMORY_TAG_UNKNOWN) 
    {
        MWARN("Mfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if (state_ptr)
    {
        state_ptr->alloc_count--;
        state_ptr->tagged_allocations[tag] -= size;

        b8 result = dynamic_allocator_free(&state_ptr->allocator, block, size);
        if (!result) 
        {
            MERROR("Failed to free block from dynamic allocator.");
        }
    }
    else 
    {
        MERROR("Mfree called before memory system was initialized, or after it was shut down.");
    }
}

void* Mzero_memory(void* block, u64 size)
{
    memset(block, 0, size);
    return block;
}

void* Mcopy_memory(void* dest, const void* source, u64 size) 
{
    return memcpy(dest, source, size);
}

void* Mset_memory(void* dest, i32 value, u64 size) 
{
    return memset(dest, value, size);
}

char* get_memory_usage_str()
{
    const u64 gib = 1024 * 1024 * 1024;
    const u32 mib = 1024 * 1024;
    const u32 kib = 1024;
    char buffer[8000] = "System memory use (tagged):\n";
    u64 total = 0;

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) 
    {
        u64 memory = state_ptr->tagged_allocations[i];
        total += memory;
        char unit[4] = "XiB";
        f32 amount = 1.0f;

        if (memory >= gib) 
        {
            unit[0] = 'G';
            amount = memory / (f32)gib;
        }
        else if (memory >= mib) 
        {
            unit[0] = 'M';
            amount = memory / (f32)mib;
        }
        else if (memory >= kib) 
        {
            unit[0] = 'K';
            amount = memory / (f32)kib;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)memory;
        }

        char line[128];
        sprintf_s(line, 128, "  %s: %.2f %s\n", memory_tag_string[i], amount, unit);
        strcat_s(buffer, 8000, line);
    }

    char total_line[128];
    sprintf_s(total_line, 128, "---------------------------\n  TOTAL: %llu Bytes\n", total);
    strcat_s(buffer, 8000, total_line);
    char* out_string = (char*)platform_allocate(strlen(buffer) + 1, FALSE);
    strcpy_s(out_string, strlen(buffer) + 1, buffer);
    return out_string;
}