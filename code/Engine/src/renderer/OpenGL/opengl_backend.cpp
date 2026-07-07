#include "opengl_backend.h"
#include "core/logger.h"
#include <glad.h>
#include <SDL.h>
#include "Platform.h"
#include "M_object_shader.h"
#include "render_buffer.h"
#include "opengl_image.h"

typedef struct opengl_geometry_data
{
    u32 vertex_buffer_offset;
    u32 index_buffer_offset;
    u32 vertex_count;
    u32 index_count;
} opengl_geometry_data;


typedef struct opengl_state
{
    M_obj_shader obj_shader;
    mat4 projection;
    mat4 view;
    render_buffer global_ubo;
    render_buffer object_vertex_buffer;
    render_buffer object_index_buffer;
    u64 geometry_vertex_offset;
    u64 geometry_index_offset;
    u32 master_vao;
    f32 fov;
    f32 near_clip;
	f32 far_clip;
    opengl_geometry_data geometries[4096];
    u32 next_internal_geometry_id;

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
	state_ptr->fov = deg_to_rad(90.0f);
	state_ptr->near_clip = 0.1f;
	state_ptr->far_clip = 1000.0f;
    state_ptr->view = mat4_id();

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
    if (!render_buffer_create(&state_ptr->object_vertex_buffer, RENDER_BUFFER_TYPE_VERTEX, vertex_buffer_size, sizeof(vertex_3d)))
    {
        MERROR("Failed to create global object vertex buffer!");
        return FALSE;
    }
    MINFO("Global object vertex buffer created with size: %llu bytes", vertex_buffer_size);
    state_ptr->geometry_vertex_offset = 0;

    u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!render_buffer_create(&state_ptr->object_index_buffer, RENDER_BUFFER_TYPE_INDEX, index_buffer_size, sizeof(u32)))
    {
        MERROR("Failed to create global object index buffer!");
        return FALSE;
    }
	//checks if it is created and how much memory it can hold
	MINFO("Global object index buffer created with size: %llu bytes", index_buffer_size);
    state_ptr->geometry_index_offset = 0;

    u64 ubo_size = sizeof(mat4) * 2;
    if (!render_buffer_create(&state_ptr->global_ubo, RENDER_BUFFER_TYPE_UNIFORM, ubo_size, sizeof(mat4)))
    {
        MERROR("Failed to create global uniform buffer!");
        return FALSE;
    }
    MINFO("Global Uniform Buffer Object created with size: %llu bytes", ubo_size);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, state_ptr->global_ubo.buffer_id);
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
    

    state_ptr->next_internal_geometry_id = 0;

    opengl_backend_resized(backend, 1280, 720);

    return TRUE;
}

//cleanup handled by platform_SDL2.cpp
void opengl_backend_shutdown(renderer_backend* backend) 
{
    if (state_ptr) {
        render_buffer_destroy(&state_ptr->global_ubo);
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
        f32 fov = deg_to_rad(90.0f);
        state_ptr->projection = mat4_perspective(state_ptr->fov, aspect, state_ptr->near_clip, state_ptr->far_clip);
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
    state_ptr->obj_shader.M_obj_shader_use();
    glBindVertexArray(state_ptr->master_vao);
    
    return TRUE;
}

b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time) 
{
    glBindVertexArray(0);
    sdl_internal_state* sdl_state = (sdl_internal_state*)backend->plat_state->internal_state;
    SDL_GL_SwapWindow(sdl_state->window);
    return TRUE;
}

void opengl_backend_update_global_state(struct renderer_backend* backend, mat4 projection, mat4 view)
{
    // state_ptr is your internal backend state holding the UBO and shader
    if (state_ptr)
    {
        // Bind the shader and stream the matrices directly into your global UBO
        state_ptr->obj_shader.M_obj_shader_use();
        state_ptr->obj_shader.update_glob_state(&state_ptr->global_ubo, state_ptr->projection, state_ptr->view);
    }
}

void opengl_backend_update_object(struct renderer_backend* backend, geometry_render_data data) {
    if (!state_ptr || !data.geometry) return;

    if (data.geometry->material && data.geometry->material->diffuse_map.texture)
    {
        glActiveTexture(GL_TEXTURE0);
        opengl_image* img = (opengl_image*)data.geometry->material->diffuse_map.texture->internal_data;
        glBindTexture(GL_TEXTURE_2D, img ? img->handle : 0);
    }

    u32 sampler_location = glGetUniformLocation(state_ptr->obj_shader.shader_id, "diffuse_sampler");
    glUniform1i(sampler_location, 0);
    state_ptr->obj_shader.update_object_state(data.model, mat4_id());
    opengl_geometry_data* internal_data = &state_ptr->geometries[data.geometry->internal_id];
    glDrawElementsBaseVertex(GL_TRIANGLES, internal_data->index_count, GL_UNSIGNED_INT, (void*)(u64)internal_data->index_buffer_offset, internal_data->vertex_buffer_offset / sizeof(vertex_3d));
}

void opengl_backend_set_projection_params(struct renderer_backend* backend, f32 fov, f32 near_clip, f32 far_clip)
{
    if (state_ptr)
    {
        state_ptr->fov = fov;
        state_ptr->near_clip = near_clip;
        state_ptr->far_clip = far_clip;

        sdl_internal_state* sdl_state = (sdl_internal_state*)backend->plat_state->internal_state;
        int width, height;
        SDL_GetWindowSize(sdl_state->window, &width, &height);

        opengl_backend_resized(backend, (u16)width, (u16)height);
    }
}

void opengl_backend_set_view(struct renderer_backend* backend, mat4 view)
{
    if (state_ptr)
    {
        state_ptr->view = view;
    }
}

void opengl_backend_create_texture(struct renderer_backend* backend, const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, struct texture* out_texture)
{
    out_texture->width = width;
    out_texture->height = height;
    out_texture->channel_count = channel_count;
    out_texture->generation = 0;
    out_texture->has_transparency = has_transparency;

    opengl_image* internal_data = (opengl_image*)Mallocate(sizeof(opengl_image), MEMORY_TAG_RENDERER);
    u32 format = GL_RGBA;
    opengl_image_create(width, height, format, TRUE, internal_data);

    if (pixels)
    {
        glBindTexture(GL_TEXTURE_2D, internal_data->handle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    out_texture->internal_data = internal_data;
    out_texture->generation++;
}

void opengl_backend_destroy_texture(struct renderer_backend* backend, struct texture* texture)
{
    if (texture && texture->internal_data)
    {
        opengl_image* internal_data = (opengl_image*)texture->internal_data;
        opengl_image_destroy(internal_data);
        Mfree(internal_data, sizeof(opengl_image), MEMORY_TAG_RENDERER);
        texture->internal_data = 0;
        Mzero_memory(texture, sizeof(struct texture));
    }
}

void opengl_backend_create_geometry(struct renderer_backend* backend, geometry* geometry, u32 vertex_count, const void* vertices, u32 index_count, const void* indices) 
{
    u32 internal_id = state_ptr->next_internal_geometry_id++;
    geometry->internal_id = internal_id;
    u32 vertex_size = sizeof(vertex_3d) * vertex_count;
    u32 index_size = sizeof(u32) * index_count;
    opengl_geometry_data* internal_data = &state_ptr->geometries[internal_id];
    internal_data->vertex_buffer_offset = state_ptr->geometry_vertex_offset;
    internal_data->index_buffer_offset = state_ptr->geometry_index_offset;
    internal_data->vertex_count = vertex_count;
    internal_data->index_count = index_count;

    // Stream new memory into global buffers!
    render_buffer_load_data(&state_ptr->object_vertex_buffer, internal_data->vertex_buffer_offset, vertex_size, vertices);
    state_ptr->geometry_vertex_offset += vertex_size;

    if (index_count > 0) 
    {
        render_buffer_load_data(&state_ptr->object_index_buffer, internal_data->index_buffer_offset, index_size, indices);
        state_ptr->geometry_index_offset += index_size;
    }
}

void opengl_backend_destroy_geometry(struct renderer_backend* backend, geometry* geometry) 
{
    //todo:implement a free list for geometry
    geometry->internal_id = 0;
}