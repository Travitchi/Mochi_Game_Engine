#include "image_loader.h"
#include "logger.h"
#include "M_memory.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

b8 image_loader_load(const char* name, void* loader, resource* out_resource) 
{
    resource_loader* typed_loader = (resource_loader*)loader;

    char full_path[512];
    sprintf_s(full_path, 512, "%s/%s/%s.png", resource_system_base_path(), typed_loader->type_path, name);

    stbi_set_flip_vertically_on_load(TRUE);
    int width, height, channels;
    u8* pixels = stbi_load(full_path, &width, &height, &channels, 4);

    if (!pixels)
    {
        MERROR("Image Loader failed to load file: %s", full_path);
        return FALSE;
    }

    image_resource_data* resource_data = (image_resource_data*)Mallocate(sizeof(image_resource_data), MEMORY_TAG_ARRAY);
    resource_data->pixels = pixels;
    resource_data->width = width;
    resource_data->height = height;
    resource_data->channel_count = 4;

    out_resource->data = resource_data;
    out_resource->data_size = sizeof(image_resource_data);
    out_resource->name = name;

    return TRUE;
}

void image_loader_unload(void* loader, resource* res)
{
    if (res && res->data) 
    {
        image_resource_data* data = (image_resource_data*)res->data;
        stbi_image_free(data->pixels);
        Mfree(data, sizeof(image_resource_data), MEMORY_TAG_ARRAY);
        res->data = 0;
    }
}

resource_loader image_resource_loader_create()
{
    resource_loader loader = {};
    loader.type = RESOURCE_TYPE_IMAGE;
    loader.type_path = "textures";
    loader.load = image_loader_load;
    loader.unload = image_loader_unload;
    return loader;
}