#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "opengl_backend.h"
#include "logger.h"
#include "M_memory.h"
#include "Platform.h"
#include "M_math.h"
#include "event.h"
#include <stdio.h>
#include "texture_systems.h"
#include "material_systems.h"
#include "geometry_systems.h"

static renderer_backend* backend = 0;
static geometry* test_geometry = 0;


b8 event_on_debug_event(u16 code, void* sender, void* listener_inst, event_context context)
{

    static f64 last_swap_time = 0;
    f64 current_time = platform_get_absolute_time();

    if (current_time - last_swap_time < 0.2) // 0.2 seconds cooldown
    {
        return TRUE; // Ignore the ghost press!
    }
    last_swap_time = current_time;
    const char* names[2] = { "test_mat_1", "test_mat_2" };
    static i32 choice = 1;

    if (test_geometry)
    {
        test_geometry->material = material_system_acquire(names[choice]);
    }

    choice++;
    choice %= 2;

    return TRUE;
}


b8 renderer_initialize(const char* application_name, struct platform_state* plat_state)
{
    backend = (renderer_backend*)Mallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
    if (!backend) 
    {
        MFATAL("Failed to allocate memory for renderer backend!");
        return FALSE;
    }
    backend->plat_state = plat_state;
    backend->frame_number = 0;

    backend->initialize = opengl_backend_initialize;
    backend->shutdown = opengl_backend_shutdown;
    backend->resized = opengl_backend_resized;
    backend->begin_frame = opengl_backend_begin_frame;
    backend->end_frame = opengl_backend_end_frame;
    backend->update_global_state = opengl_backend_update_global_state;
    backend->update_object = opengl_backend_update_object;
    backend->set_projection_params = opengl_backend_set_projection_params;
    backend->set_view = opengl_backend_set_view;
    backend->create_texture = opengl_backend_create_texture;
    backend->destroy_texture = opengl_backend_destroy_texture;
    backend->create_geometry = opengl_backend_create_geometry;
    backend->destroy_geometry = opengl_backend_destroy_geometry;

    if (!backend->initialize(backend, application_name, plat_state))
    {
        MFATAL("Renderer backend failed to initialize!");
        return FALSE;
    }

    event_register(0x10, 0, event_on_debug_event);
    return TRUE;
}

void renderer_shutdown()
{
    if (backend) 
    {
        backend->shutdown(backend);

        Mfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
        backend = 0;
    }
}

void renderer_on_resize(u16 width, u16 height) {
    if (backend) {
        backend->resized(backend, width, height);
    }
}


void renderer_set_view(mat4 view)
{
    if (backend && backend->set_view)
    {
        backend->set_view(backend, view);
    }
}

b8 renderer_draw_frame(render_packet* packet)
{
    if (backend->begin_frame(backend, packet->delta_time))
    {
		//test rotation
        if (backend->update_global_state)
        {
            backend->update_global_state(backend, mat4_id(), mat4_id());
        }

        if (backend->update_object) 
        {
            mat4 translation = mat4_translation(vect3{ 0.0f, -2.0f, -10.0f });
            geometry_render_data data = {};

            if (!test_geometry) 
            {
                geometry_config plane_config = geometry_system_generate_plane_config(10.0f, 5.0f, 5, 5, 2.0f, 2.0f, "test_plane", "test_mat_1");
                test_geometry = geometry_system_acquire_from_config(plane_config, TRUE);
                Mfree(plane_config.vertices, plane_config.vertex_size, MEMORY_TAG_ARRAY);
                Mfree(plane_config.indices, plane_config.index_size, MEMORY_TAG_ARRAY);
            }
            data.geometry = test_geometry;
            data.model = translation;
            backend->update_object(backend, data);
        }

        b8 result = backend->end_frame(backend, packet->delta_time);
        if (!result)
        {
            MERROR("renderer_end_frame failed. Shutting down.");
            return FALSE;
        }

        backend->frame_number++;
    }

    return TRUE;
}

void renderer_create_texture(const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, texture* out_texture)
{
    if (backend && backend->create_texture) {
        backend->create_texture(backend, name, auto_release, width, height, channel_count, pixels, has_transparency, out_texture);
    }
}

void renderer_destroy_texture(texture* texture)
{
    if (backend && backend->destroy_texture) {
        backend->destroy_texture(backend, texture);
    }
}

void renderer_create_geometry(geometry* geometry, u32 vertex_count, const void* vertices, u32 index_count, const void* indices)
{
    if (backend && backend->create_geometry) 
    {
        backend->create_geometry(backend, geometry, vertex_count, vertices, index_count, indices);
    }
}

void renderer_destroy_geometry(geometry* geometry) 
{
    if (backend && backend->destroy_geometry) 
    {
        backend->destroy_geometry(backend, geometry);
    }
}
