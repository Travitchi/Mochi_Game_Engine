#include "opengl_backend.h"
#include "core/logger.h"
#include <glad.h>
#include <SDL.h>
#include "Platform.h"
#include "M_object_shader.h"


typedef struct opengl_state
{
    M_obj_shader obj_shader;
} opengl_state;

static opengl_state* state_ptr;

typedef struct sdl_internal_state {
    SDL_Window* window;
    SDL_GLContext gl_context;
} sdl_internal_state;


void APIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id,GLenum severity, GLsizei length,const GLchar* message, const void* userParam) 
{
    if (severity == GL_DEBUG_SEVERITY_HIGH) 
    {
        MERROR("OpenGL Error: %s", message);
    }
    else {
        //add more logic here for warnings or info
    }
}

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state)
{

    state_ptr = (opengl_state*)Mallocate(sizeof(opengl_state), MEMORY_TAG_RENDERER);
    Mzero_memory(state_ptr, sizeof(opengl_state));

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        MERROR("Failed to initialize GLAD");
        return FALSE;
    }


    MINFO("OpenGL Backend successfully initialized!");
    //GPU INFO for troubleshooting
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    MINFO("OpenGL Renderer Info:");
    MINFO("  Vendor: %s", vendor);
    MINFO("  Renderer: %s", renderer);
    MINFO("  Version: %s", version);
    //
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_debug_message_callback, NULL);

    state_ptr->obj_shader.M_obj_shader_init();
    return TRUE;
}

//cleanup handled by platform_SDL2.cpp
void opengl_backend_shutdown(renderer_backend* backend) 
{
    if (state_ptr) {
        glDeleteProgram(state_ptr->obj_shader.shader_id);
        Mfree(state_ptr, sizeof(opengl_state), MEMORY_TAG_RENDERER);
        state_ptr = nullptr;
    }

}


void opengl_backend_resized(renderer_backend* backend, u16 width, u16 height) 
{
    glViewport(0, 0, width, height);
}

b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time) 
{
    static b8 first_frame = TRUE;
    if (first_frame) {
        MINFO("First frame successfully cleared! Backend is working.");
        first_frame = FALSE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return TRUE;
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time) 
{
    sdl_internal_state* sdl_state = (sdl_internal_state*)backend->plat_state->internal_state;
    SDL_GL_SwapWindow(sdl_state->window);
    return TRUE;
}