#include "freelist.h"
#include "logger.h"
#include "M_memory.h"

//flag a unused node array slot
#define INVALID_ID_U64 0xFFFFFFFFFFFFFFFFULL

typedef struct freelist_node 
{
    u64 offset;
    u64 size;
    struct freelist_node* next;
} freelist_node;

typedef struct freelist_state
{
    u64 total_size;
    u32 max_entries;
    freelist_node* head;
    freelist_node* nodes;
} freelist_state;

//helper functions 
static freelist_node* get_node(freelist_state* state) 
{
    for (u32 i = 0; i < state->max_entries; ++i)
    {
        if (state->nodes[i].offset == INVALID_ID_U64) 
        {
            return &state->nodes[i];
        }
    }
    return 0;
}

static void return_node(freelist_state* state, freelist_node* node)
{
    node->offset = INVALID_ID_U64;
    node->size = INVALID_ID_U64;
    node->next = 0;
}

void freelist_create(u64 total_size, u64* memory_requirement, void* memory, freelist* out_list) {
    // Determine maximum possible nodes.
    u32 max_entries = (u32)(total_size / sizeof(void*));
    *memory_requirement = sizeof(freelist_state) + (sizeof(freelist_node) * max_entries);
    if (!memory) return;

    out_list->memory = memory;
    freelist_state* state = (freelist_state*)out_list->memory;
    state->nodes = (freelist_node*)((u8*)out_list->memory + sizeof(freelist_state));
    state->max_entries = max_entries;
    state->total_size = total_size;
    Mzero_memory(state->nodes, sizeof(freelist_node) * state->max_entries);
    state->head = &state->nodes[0];
    state->head->offset = 0;
    state->head->size = total_size;
    state->head->next = 0;

    for (u32 i = 1; i < state->max_entries; ++i) 
    {
        state->nodes[i].offset = INVALID_ID_U64;
        state->nodes[i].size = INVALID_ID_U64;
    }
}

void freelist_destroy(freelist* list)
{
    if (list && list->memory) 
    {
        list->memory = 0;
    }
}

b8 freelist_allocate_block(freelist* list, u64 size, u64* out_offset)
{
    if (!list || !list->memory || !out_offset) return FALSE;
    freelist_state* state = (freelist_state*)list->memory;
    freelist_node* node = state->head;
    freelist_node* previous = 0;

    while (node) 
    {
        if (node->size == size) 
        {
            *out_offset = node->offset;
            freelist_node* node_to_return = 0;
            if (previous)
            {
                previous->next = node->next;
                node_to_return = node;
            }
            else 
            {
                node_to_return = state->head;
                state->head = node->next;
            }
            return_node(state, node_to_return);
            return TRUE;
        }
        else if (node->size > size) 
        {
            *out_offset = node->offset;
            node->size -= size;
            node->offset += size;
            return TRUE;
        }

        previous = node;
        node = node->next;
    }

    MWARN("freelist_allocate_block - No block found with enough free space (Requested: %llu)", size);
    return FALSE;
}

b8 freelist_free_block(freelist* list, u64 size, u64 offset)
{
    if (!list || !list->memory) return FALSE;

    freelist_state* state = (freelist_state*)list->memory;
    freelist_node* node = state->head;
    freelist_node* previous = 0;

    freelist_node* new_node = get_node(state);
    if (!new_node)
    {
        MERROR("freelist_free_block - Failed to get a free internal node.");
        return FALSE;
    }

    new_node->offset = offset;
    new_node->size = size;
    new_node->next = 0;

    // Scan for the correct sorted position to insert the freed block
    while (node)
    {
        if (node->offset > offset)
        {
            new_node->next = node;
            if (previous)
            {
                previous->next = new_node;
            }
            else 
            {
                state->head = new_node;
            }
            
            if (previous && previous->offset + previous->size == new_node->offset)
            {
                previous->size += new_node->size;
                previous->next = new_node->next;
                return_node(state, new_node);
                new_node = previous;
            }

            if (new_node->next && new_node->offset + new_node->size == new_node->next->offset)
            {
                freelist_node* next_node = new_node->next;
                new_node->size += next_node->size;
                new_node->next = next_node->next;
                return_node(state, next_node);
            }

            return TRUE;
        }
        previous = node;
        node = node->next;
    }

    if (previous) 
    {
        previous->next = new_node;
        if (previous->offset + previous->size == new_node->offset) 
        {
            previous->size += new_node->size;
            previous->next = new_node->next;
            return_node(state, new_node);
        }
    }
    else 
    {
        state->head = new_node;
    }

    return TRUE;
}

void freelist_clear(freelist* list)
{
    if (!list || !list->memory) return;
    freelist_state* state = (freelist_state*)list->memory;
    for (u32 i = 1; i < state->max_entries; ++i)
    {
        state->nodes[i].offset = INVALID_ID_U64;
        state->nodes[i].size = INVALID_ID_U64;
    }

    state->head = &state->nodes[0];
    state->head->offset = 0;
    state->head->size = state->total_size;
    state->head->next = 0;
}

u64 freelist_free_space(freelist* list) 
{
    if (!list || !list->memory) return 0;

    freelist_state* state = (freelist_state*)list->memory;
    u64 total_free = 0;
    freelist_node* node = state->head;

    while (node) 
    {
        total_free += node->size;
        node = node->next;
    }

    return total_free;
}

b8 freelist_resize(freelist* list, u64* memory_requirement, void* new_memory, u64 new_size, void** out_old_memory)
{
    if (!list || new_size <= 0) return FALSE;
    freelist_state* old_state = (freelist_state*)list->memory;
    u64 old_size = old_state->total_size;
    u32 new_max_entries = (u32)(new_size / sizeof(void*));
    *memory_requirement = sizeof(freelist_state) + (sizeof(freelist_node) * new_max_entries);
    if (!new_memory) return TRUE;

    if (new_size <= old_size) 
    {
        MERROR("freelist_resize - new_size must be greater than old size.");
        return FALSE;
    }

    freelist_state* new_state = (freelist_state*)new_memory;
    new_state->nodes = (freelist_node*)((u8*)new_memory + sizeof(freelist_state));
    new_state->max_entries = new_max_entries;
    new_state->total_size = new_size;

    for (u32 i = 0; i < new_state->max_entries; ++i) 
    {
        new_state->nodes[i].offset = INVALID_ID_U64;
        new_state->nodes[i].size = INVALID_ID_U64;
        new_state->nodes[i].next = 0;
    }

    Mcopy_memory(new_state->nodes, old_state->nodes, sizeof(freelist_node) * old_state->max_entries);
    if (old_state->head)
    {
        u32 head_idx = (u32)(old_state->head - old_state->nodes);
        new_state->head = &new_state->nodes[head_idx];
    }
    else 
    {
        new_state->head = 0;
    }

    for (u32 i = 0; i < old_state->max_entries; ++i)
    {
        if (new_state->nodes[i].offset != INVALID_ID_U64 && new_state->nodes[i].next)
        {
            freelist_node* old_next_ptr = new_state->nodes[i].next;
            u32 next_idx = (u32)(old_next_ptr - old_state->nodes);
            new_state->nodes[i].next = &new_state->nodes[next_idx];
        }
    }

    *out_old_memory = list->memory;
    list->memory = new_memory;
    u64 size_diff = new_size - old_size;
    freelist_free_block(list, size_diff, old_size);

    return TRUE;
}