#include "sprite_manager_system.h"
#include "M_memory.h"
#include "logger.h"
#include "texture_systems.h"
#include <string.h>

typedef struct sprite_manager_system_state 
{
    sprite_manager_system_config config;
    sprite_sheet* registered_sheets;
    sprite* registered_sprites;
    u32 sheet_count;
    u32 sprite_count;
} sprite_manager_system_state;

static sprite_manager_system_state* state_ptr = 0;

b8 sprite_system_initialize(u64* memory_requirement, void* state, sprite_manager_system_config config)
{
    if (config.max_sprite_sheet_count == 0 || config.max_sprite_count == 0)
    {
        MERROR("sprite_system_initialize failed: max counts must be greater than 0.");
        return FALSE;
    }

    u64 struct_requirement = sizeof(sprite_manager_system_state);
    u64 sheets_requirement = sizeof(sprite_sheet) * config.max_sprite_sheet_count;
    u64 sprites_requirement = sizeof(sprite) * config.max_sprite_count;
    *memory_requirement = struct_requirement + sheets_requirement + sprites_requirement;
    if (!state) return TRUE;
    state_ptr = (sprite_manager_system_state*)state;
    state_ptr->config = config;
    state_ptr->sheet_count = 0;
    state_ptr->sprite_count = 0;

    void* sheets_block = (u8*)state + struct_requirement;
    state_ptr->registered_sheets = (sprite_sheet*)sheets_block;
    void* sprites_block = (u8*)sheets_block + sheets_requirement;
    state_ptr->registered_sprites = (sprite*)sprites_block;

    Mzero_memory(state_ptr->registered_sheets, sheets_requirement);
    Mzero_memory(state_ptr->registered_sprites, sprites_requirement);

    MINFO("Sprite System successfully initialized (Max Sheets: %u, Max Sprites: %u).", config.max_sprite_sheet_count, config.max_sprite_count);
    return TRUE;
}

void sprite_system_shutdown(void* state)
{
    if (state_ptr)
    {
        for (u32 i = 0; i < state_ptr->sheet_count; ++i)
        {
            sprite_system_destroy_sheet(&state_ptr->registered_sheets[i]);
        }
        state_ptr = 0;
    }
}

sprite_sheet* sprite_system_create_sheet(const char* name, const char* diffuse_texture_name, const char* normal_texture_name, u32 tile_width, u32 tile_height)
{
    if (!state_ptr || state_ptr->sheet_count >= state_ptr->config.max_sprite_sheet_count)
    {
        MERROR("Sprite sheet capacity reached or system not initialized!");
        return 0;
    }

    if (tile_width == 0 || tile_height == 0)
    {
        MERROR("Cannot create sprite sheet '%s' with 0 tile dimensions!", name);
        return 0;
    }

    u32 id = state_ptr->sheet_count++;
    sprite_sheet* sheet = &state_ptr->registered_sheets[id];
    sheet->id = id;
    strcpy_s(sheet->name, 256, name);

    // Acquire Diffuse Texture
    sheet->diffuse_texture = texture_system_acquire_sprite(diffuse_texture_name, TRUE);
    if (!sheet->diffuse_texture) 
    {
        MWARN("Failed to acquire diffuse texture '%s' for sheet '%s'. Using default.", diffuse_texture_name, name);
        sheet->diffuse_texture = texture_system_get_default_texture();
    }
    
    if (normal_texture_name && strlen(normal_texture_name) > 0) 
    {
        sheet->normal_texture = texture_system_acquire_sprite(normal_texture_name, TRUE);
    } 
    else 
    {
        sheet->normal_texture = texture_system_get_default_texture();
    }

    sheet->width = sheet->diffuse_texture->width;
    sheet->height = sheet->diffuse_texture->height;
    sheet->tile_width = tile_width;
    sheet->tile_height = tile_height;

    sheet->columns = sheet->width / tile_width;
    sheet->rows = sheet->height / tile_height;

    if (sheet->columns == 0) sheet->columns = 1;
    if (sheet->rows == 0) sheet->rows = 1;

    MINFO("Created sprite sheet '%s' (%ux%u grid, %u total tiles).", name, sheet->columns, sheet->rows, sheet->columns * sheet->rows);
    return sheet;
}

void sprite_system_destroy_sheet(sprite_sheet* sheet)
{
    if (sheet && sheet->diffuse_texture)
    {
        texture_system_release(sheet->diffuse_texture->name);
        if (sheet->normal_texture)
        {
            texture_system_release(sheet->normal_texture->name);
        }
        Mzero_memory(sheet, sizeof(sprite_sheet));
    }
}

sprite_sheet* sprite_system_get_sheet(const char* name)
{
    if (!state_ptr) return 0;
    for (u32 i = 0; i < state_ptr->sheet_count; ++i)
    {
        if (strcmp(state_ptr->registered_sheets[i].name, name) == 0)
        {
            return &state_ptr->registered_sheets[i];
        }
    }
    MWARN("Sprite sheet '%s' not found.", name);
    return 0;
}

sprite* sprite_system_create_sprite(const char* name, sprite_sheet* sheet)
{
    if (!state_ptr || !sheet || state_ptr->sprite_count >= state_ptr->config.max_sprite_count)
    {
        MERROR("Sprite capacity reached or invalid sheet provided for '%s'!", name);
        return 0;
    }

    u32 id = state_ptr->sprite_count++;
    sprite* s = &state_ptr->registered_sprites[id];
    s->id = id;
    strcpy_s(s->name, 256, name);
    s->sheet = sheet;

    // Initialize to frame 0
    sprite_set_frame(s, 0);
    return s;
}

void sprite_system_destroy_sprite(sprite* s)
{
    if (s)
    {
        Mzero_memory(s, sizeof(sprite));
    }
}

sprite* sprite_system_get_sprite(const char* name)
{
    if (!state_ptr) return 0;
    for (u32 i = 0; i < state_ptr->sprite_count; ++i)
    {
        if (strcmp(state_ptr->registered_sprites[i].name, name) == 0)
        {
            return &state_ptr->registered_sprites[i];
        }
    }
    MWARN("Sprite '%s' not found.", name);
    return 0;
}

void sprite_set_frame(sprite* s, u32 frame_index)
{
    if (!s || !s->sheet) return;
    u32 total_frames = s->sheet->columns * s->sheet->rows;
    if (total_frames == 0) return;
    // Safely wrap out-of-bounds frame indices
    if (frame_index >= total_frames)
    {
        frame_index = frame_index % total_frames;
    }

    s->current_frame = frame_index;
    // Calculate 2D grid coordinates
    u32 col = frame_index % s->sheet->columns;
    u32 row = frame_index / s->sheet->columns;
    // Calculate normalized UV scale (size of one tile in 0.0 to 1.0 space)
    s->uv_scale = vect2_create(1.0f / (f32)s->sheet->columns, 1.0f / (f32)s->sheet->rows);
    // Calculate normalized UV offset (starting position of the tile)
    s->uv_offset = vect2_create((f32)col * s->uv_scale.x, (f32)row * s->uv_scale.y);
    // Calculate exact bounding box coordinates
    s->uv_min = s->uv_offset;
    s->uv_max = vect2_create(s->uv_min.x + s->uv_scale.x, s->uv_min.y + s->uv_scale.y);
}

void sprite_set_frame_by_coord(sprite* s, u32 row, u32 col)
{
    if (!s || !s->sheet) return;
    // Safety clamp: Ensure we don't request a column or row that doesn't exist
    //if (col >= s->sheet->columns) col = s->sheet->columns - 1;
   // if (row >= s->sheet->rows) row = s->sheet->rows - 1;
    // Convert the 2D coordinates into your engine's 1D frame index
    u32 calculated_frame_index = (row * s->sheet->columns) + col;
    // Call your existing function to do the actual UV math
    sprite_set_frame(s, calculated_frame_index);
}