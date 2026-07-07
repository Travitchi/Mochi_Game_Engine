#include "material_loader.h"
#include "core/logger.h"
#include "core/M_memory.h"
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>

b8 material_loader_load(const char* name, void* loader, resource* out_resource) 
{
    resource_loader* typed_loader = (resource_loader*)loader;
    char full_path[512];
    sprintf_s(full_path, 512, "%s/%s/%s.kmt", resource_system_base_path(), typed_loader->type_path, name);

    std::ifstream file(full_path);
    if (!file.is_open()) 
    {
        MERROR("Material Loader failed to open file: %s", full_path);
        return FALSE;
    }

    material_config* config = (material_config*)Mallocate(sizeof(material_config), MEMORY_TAG_ARRAY);
    strcpy_s(config->name, 256, name);
    config->auto_release = TRUE;
    config->diffuse_color = vect4_create(1.0f, 1.0f, 1.0f, 1.0f);
    config->diffuse_map_name[0] = '\0';

    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#') continue;
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) continue;

        std::string key = line.substr(0, equal_pos);
        std::string value = line.substr(equal_pos + 1);

        if (key == "diffuse_map_name") 
        {
            strcpy_s(config->diffuse_map_name, 256, value.c_str());
        }
        else if (key == "diffuse_color")
        {
            sscanf_s(value.c_str(), "%f %f %f %f", &config->diffuse_color.x, &config->diffuse_color.y, &config->diffuse_color.z, &config->diffuse_color.w);
        }
    }
    file.close();

    out_resource->data = config;
    out_resource->data_size = sizeof(material_config);
    out_resource->name = name;
    return TRUE;
}

void material_loader_unload(void* loader, resource* res) 
{
    if (res && res->data) {
        Mfree(res->data, sizeof(material_config), MEMORY_TAG_ARRAY);
        res->data = 0;
    }
}

resource_loader material_resource_loader_create() 
{
    resource_loader loader = {};
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.type_path = "materials";
    loader.load = material_loader_load;
    loader.unload = material_loader_unload;
    return loader;
}