#include "hashtable.h"
#include "M_memory.h"
#include "logger.h"

static u64 hash_name(const char* name, u32 element_count)
{
    u64 hash = 5381;
    int c;
    while ((c = *name++)) 
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % element_count;
}

void hashtable_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hashtable* out_hashtable)
{
    if (!memory || !out_hashtable) return;
    out_hashtable->memory = memory;
    out_hashtable->element_size = element_size;
    out_hashtable->element_count = element_count;
    out_hashtable->is_pointer_type = is_pointer_type;
    Mzero_memory(out_hashtable->memory, element_size * element_count);
}

void hashtable_destroy(hashtable* table) 
{
    if (table) Mzero_memory(table, sizeof(hashtable));
}

b8 hashtable_set_ptr(hashtable* table, const char* name, void** value) 
{
    if (!table || !name || !table->is_pointer_type) return FALSE;
    u64 hash = hash_name(name, table->element_count);
    void** dest = (void**)((u8*)table->memory + (table->element_size * hash));
    *dest = value ? *value : 0;
    return TRUE;
}

b8 hashtable_get_ptr(hashtable* table, const char* name, void** out_value)
{
    if (!table || !name || !out_value || !table->is_pointer_type) return FALSE;
    u64 hash = hash_name(name, table->element_count);
    void** src = (void**)((u8*)table->memory + (table->element_size * hash));
    *out_value = *src;
    return *out_value != 0;
}

b8 hashtable_fill(hashtable* table, void* value) 
{
    if (!table || !value) return FALSE;
    for (u32 i = 0; i < table->element_count; ++i) 
    {
        void* dest = (u8*)table->memory + (table->element_size * i);
        Mcopy_memory(dest, value, table->element_size);
    }
    return TRUE;
}

b8 hashtable_set(hashtable* table, const char* name, void* value) 
{
    if (!table || !name || !value) return FALSE;

    u64 hash = 5381;
    i32 c;
    const char* str = name;
    while ((c = *str++)) 
    {
        hash = ((hash << 5) + hash) + c;
    }

    hash %= table->element_count;
    Mcopy_memory((u8*)table->memory + (table->element_size * hash), value, table->element_size);
    return TRUE;
}

b8 hashtable_get(hashtable* table, const char* name, void* out_value)
{
    if (!table || !name || !out_value) return FALSE;
    u64 hash = 5381;
    i32 c;
    const char* str = name;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    hash %= table->element_count;
    Mcopy_memory(out_value, (u8*)table->memory + (table->element_size * hash), table->element_size);
    return TRUE;
}