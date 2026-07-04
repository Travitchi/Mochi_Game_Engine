#include "opengl_backend.h"
#include "core/logger.h"
#include <glad.h>
#include <SDL.h>
#include "Platform.h"
#include "M_object_shader.h"
#include "render_buffer.h"


typedef struct opengl_state
{
    M_obj_shader obj_shader;
    mat4 projection;
    render_buffer object_vertex_buffer;
    render_buffer object_index_buffer;
    u64 geometry_vertex_offset;
    u64 geometry_index_offset;
    u32 master_vao;
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
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_debug_message_callback, NULL);

    state_ptr->obj_shader.M_obj_shader_init();

    u64 vertex_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!render_buffer_create(&state_ptr->object_vertex_buffer, vertex_buffer_size, sizeof(vertex_3d)))
    {
        MERROR("Failed to create global object vertex buffer!");
        return FALSE;
    }
    //checks if it is created and how much memory it can hold
	MINFO("Global object vertex buffer created with size: %llu bytes", vertex_buffer_size);
    state_ptr->geometry_vertex_offset = 0;
    
    u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!render_buffer_create(&state_ptr->object_index_buffer, index_buffer_size, sizeof(u32)))
    {
        MERROR("Failed to create global object index buffer!");
        return FALSE;
    }
	//checks if it is created and how much memory it can hold
	MINFO("Global object index buffer created with size: %llu bytes", index_buffer_size);
    state_ptr->geometry_index_offset = 0;

    glGenVertexArrays(1, &state_ptr->master_vao);
    glBindVertexArray(state_ptr->master_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state_ptr->object_vertex_buffer.buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state_ptr->object_index_buffer.buffer_id);

    //Tells OpenGL how to read vertex_3d struct!
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_3d), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_3d), (void*)(sizeof(f32) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_3d), (void*)(sizeof(f32) * 5));
    glEnableVertexAttribArray(2);
    //safety reasons we unbind
    glBindVertexArray(0);


    // TODO: TEMPORARY TEST CODE
    const u32 vert_count = 4;
    vertex_3d verts[vert_count];
    Mzero_memory(verts, sizeof(vertex_3d) * vert_count);

    // Bottom-middle vertex
    verts[0].position.x = 0.5f;
    verts[0].position.y = -0.5f;

    // Top-right vertex
    verts[1].position.x = 0.5f;
    verts[1].position.y = 0.5f;

    // Top-left vertex
    verts[2].position.x = -0.5f;
    verts[2].position.y = 0.5f;

	verts[3].position.x = -0.5f;
	verts[3].position.y = -0.5f;

    const u32 index_count = 6;
	// Define the indices for the two triangles that make up the square
    u32 indices[index_count] = { 0, 1, 2 , 0, 2, 3};

    // 1. Upload temporary vertices to offset 0 of our global vertex buffer
    render_buffer_load_data( &state_ptr->object_vertex_buffer, 0, sizeof(vertex_3d) * vert_count, verts);

    // 2. Upload temporary indices to offset 0 of our global index buffer
    render_buffer_load_data(&state_ptr->object_index_buffer, 0, sizeof(u32) * index_count, indices);
    
    return TRUE;
}

//cleanup handled by platform_SDL2.cpp
void opengl_backend_shutdown(renderer_backend* backend) 
{
    if (state_ptr) {
        render_buffer_destroy(&state_ptr->object_vertex_buffer);
        render_buffer_destroy(&state_ptr->object_index_buffer);
        glDeleteProgram(state_ptr->obj_shader.shader_id);
        Mfree(state_ptr, sizeof(opengl_state), MEMORY_TAG_RENDERER);
        state_ptr = nullptr;
    }

}


void opengl_backend_resized(renderer_backend* backend, u16 width, u16 height) 
{
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);

    if (height != 0)
    {
        f32 aspect = (f32)width / (f32)height;
        state_ptr->projection = mat4_orthographic(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);
    }
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

    // TODO: test code

    //Activate the Shader
    state_ptr->obj_shader.M_obj_shader_use();

    mat4 view = mat4_id();
    mat4 model = mat4_id();
    state_ptr->obj_shader.update_glob_state(state_ptr->projection, view);
    state_ptr->obj_shader.update_object_state(model);

    // automatically binds the vertex & index buffers
    glBindVertexArray(state_ptr->master_vao);

    // Draws 3 indices, as triangles starting at offset 0
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //Unbind VAO for clean state
    glBindVertexArray(0);
    return TRUE;
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time) 
{
    sdl_internal_state* sdl_state = (sdl_internal_state*)backend->plat_state->internal_state;
    SDL_GL_SwapWindow(sdl_state->window);
    return TRUE;
}