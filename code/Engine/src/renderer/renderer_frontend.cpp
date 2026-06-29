#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "opengl_backend.h"
#include "logger.h"
#include "M_memory.h"
#include "Platform.h"

static renderer_backend* backend = 0;

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

    if (!backend->initialize(backend, application_name, plat_state))
    {
        MFATAL("Renderer backend failed to initialize!");
        return FALSE;
    }

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

b8 renderer_draw_frame(render_packet* packet)
{
    if (backend->begin_frame(backend, packet->delta_time))
    {
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
