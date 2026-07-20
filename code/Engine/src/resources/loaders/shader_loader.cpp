#include "shader_loader.h"
#include "logger.h"
#include "M_memory.h"
#include "shader_system.h"
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>

b8 shader_loader_load(const char* name, void* loader, resource* out_resource)
{
    resource_loader* typed_loader = (resource_loader*)loader;
    char full_path[512];
    sprintf_s(full_path, 512, "%s/%s/%s.shd", resource_system_base_path(), typed_loader->type_path, name);

    std::ifstream file(full_path);
    if (!file.is_open())
    {
        MERROR("Shader Loader failed to open file: %s", full_path);
        return FALSE;
    }

    shader_config* config = (shader_config*)Mallocate(sizeof(shader_config), MEMORY_TAG_ARRAY);
    Mzero_memory(config, sizeof(shader_config));
    strcpy_s(config->name, 256, name);
    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#') continue;
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) continue;
        std::string key = line.substr(0, equal_pos);
        std::string value = line.substr(equal_pos + 1);
        size_t comma_pos = value.find(',');
        if (comma_pos == std::string::npos) continue;
        std::string var_name = value.substr(0, comma_pos);
        std::string var_type_str = value.substr(comma_pos + 1);
        int var_type = std::stoi(var_type_str);

        if (key == "attribute")
        {
            u8 index = config->attribute_count++;
            strcpy_s(config->attributes[index].name, 256, var_name.c_str());
            config->attributes[index].type = (shader_attribute_type)var_type;
        }
        else if (key == "uniform")
        {
            u8 index = config->uniform_count++;
            strcpy_s(config->uniforms[index].name, 256, var_name.c_str());
            config->uniforms[index].type = (shader_uniform_type)var_type;
        }
    }

    file.close();
    out_resource->data = config;
    out_resource->data_size = sizeof(shader_config);
    out_resource->name = name;
    return TRUE;
}

void shader_loader_unload(void* loader, resource* res) 
{
    if (res && res->data) 
    {
        Mfree(res->data, sizeof(shader_config), MEMORY_TAG_ARRAY);
        res->data = 0;
    }
}

resource_loader shader_resource_loader_create() 
{
    resource_loader loader = {};
    loader.type = RESOURCE_TYPE_SHADER;
    loader.type_path = "shaders";
    loader.load = shader_loader_load;
    loader.unload = shader_loader_unload;
    return loader;
}