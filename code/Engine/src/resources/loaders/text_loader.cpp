#include "text_loader.h"
#include "logger.h"
#include "M_memory.h"
#include <fstream>
#include <sstream>
#include <string.h>

b8 text_loader_load(const char* name, void* loader, resource* out_resource)
{
    resource_loader* typed_loader = (resource_loader*)loader;

    char full_path[512];
    sprintf_s(full_path, 512, "%s/%s/%s", resource_system_base_path(), typed_loader->type_path, name);

    std::ifstream file(full_path);
    if (!file.is_open()) 
    {
        MERROR("Text Loader failed to open file: %s", full_path);
        return FALSE;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    char* text_data = (char*)Mallocate(content.length() + 1, MEMORY_TAG_STRING);
    strcpy_s(text_data, content.length() + 1, content.c_str());

    out_resource->data = text_data;
    out_resource->data_size = content.length() + 1;
    out_resource->name = name;

    return TRUE;
}

void text_loader_unload(void* loader, resource* res) 
{
    if (res && res->data) 
    {
        Mfree(res->data, res->data_size, MEMORY_TAG_STRING);
        res->data = 0;
    }
}

resource_loader text_resource_loader_create() 
{
    resource_loader loader = {};
    loader.type = RESOURCE_TYPE_TEXT;
    loader.type_path = "shaders";
    loader.load = text_loader_load;
    loader.unload = text_loader_unload;
    return loader;
}