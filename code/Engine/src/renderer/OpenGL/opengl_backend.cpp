#include "opengl_backend.h"
#include "core/logger.h"
#include <glad.h>
#include <SDL.h>
#include "Platform.h"
#include "render_buffer.h"
#include "opengl_image.h"
#include "opengl_shader.h"
#include "shader_system.h"
#include "light_system.h"
#include "camera_system.h"

typedef struct opengl_geometry_data
{
    u32 vertex_buffer_offset;
    u32 index_buffer_offset;
    u32 vertex_count;
    u32 index_count;
} opengl_geometry_data;


typedef struct opengl_state
{
    mat4 projection;
    mat4 view;
    render_buffer global_ubo;
    render_buffer object_vertex_buffer;
    render_buffer object_index_buffer;
    u32 master_vao;
    f32 fov;
    f32 near_clip;
	f32 far_clip;
    opengl_geometry_data geometries[4096];
    u32 next_internal_geometry_id;
    u32 ui_vao;

} opengl_state;

static opengl_state* state_ptr;

typedef struct sdl_internal_state {
    SDL_Window* window;
    SDL_GLContext gl_context;
} sdl_internal_state;

typedef struct global_uniform_data {
    mat4 projection;
    mat4 view;
    vect4 ambient_color;
    vect4 view_position;
    directional_light dir_light;
    point_light p_lights[MAX_POINT_LIGHTS];
    u32 num_p_lights;
    u32 padding[3];
} global_uniform_data;

void APIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id,GLenum severity, GLsizei length,const GLchar* message, const void* userParam) 
{
    if (severity == GL_DEBUG_SEVERITY_HIGH) 
    {
        MERROR("OpenGL Error: %s", message);
    }
    else
    {
        //add more logic here for warnings or info
    }
}

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state)
{

    state_ptr = (opengl_state*)Mallocate(sizeof(opengl_state), MEMORY_TAG_RENDERER);
    Mzero_memory(state_ptr, sizeof(opengl_state));
	state_ptr->fov = deg_to_rad(30.0f);
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

   

    u64 vertex_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!render_buffer_create(&state_ptr->object_vertex_buffer, RENDER_BUFFER_TYPE_VERTEX, vertex_buffer_size, sizeof(vertex_3d)))
    {
        MERROR("Failed to create global object vertex buffer!");
        return FALSE;
    }
    MINFO("Global object vertex buffer created with size: %llu bytes", vertex_buffer_size);
    

    u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!render_buffer_create(&state_ptr->object_index_buffer, RENDER_BUFFER_TYPE_INDEX, index_buffer_size, sizeof(u32)))
    {
        MERROR("Failed to create global object index buffer!");
        return FALSE;
    }
	//checks if it is created and how much memory it can hold
	MINFO("Global object index buffer created with size: %llu bytes", index_buffer_size);
    

    u64 ubo_size = sizeof(global_uniform_data);
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
    glBindVertexArray(0);

    //ui VAO
    glGenVertexArrays(1, &state_ptr->ui_vao);
    glBindVertexArray(state_ptr->ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state_ptr->object_vertex_buffer.buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state_ptr->object_index_buffer.buffer_id);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_2d), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_2d), (void*)(sizeof(f32) * 2));
    glEnableVertexAttribArray(1);
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
        f32 fov = deg_to_rad(35.0f);
        state_ptr->projection = mat4_perspective(state_ptr->fov, aspect, state_ptr->near_clip, state_ptr->far_clip);
    }
}

b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time) 
{
    static b8 first_frame = TRUE;
    if (first_frame)
    {
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
    glBindVertexArray(0);
    sdl_internal_state* sdl_state = (sdl_internal_state*)backend->plat_state->internal_state;
    SDL_GL_SwapWindow(sdl_state->window);
    return TRUE;
}

void opengl_backend_update_object(struct renderer_backend* backend, geometry_render_data data)
{
    if (!state_ptr || !data.geometry) return;
    b8 is_ui = (data.geometry->material && data.geometry->material->type == MATERIAL_TYPE_UI);

    if (is_ui) 
    {
        glBindVertexArray(state_ptr->ui_vao);
    }
    else 
    {
        glBindVertexArray(state_ptr->master_vao);
    }

    if (data.geometry->material && data.geometry->material->diffuse_map.texture)
    {
        opengl_image* img = (opengl_image*)data.geometry->material->diffuse_map.texture->internal_data;
        if (img)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, img->handle);
        }
    }

    if (data.geometry->material && data.geometry->material->specular_map.texture)
    {
        opengl_image* spec_img = (opengl_image*)data.geometry->material->specular_map.texture->internal_data;
        if (spec_img)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, spec_img->handle);
        }
    }

    opengl_geometry_data* internal_data = &state_ptr->geometries[data.geometry->internal_id];
    u32 stride = is_ui ? sizeof(vertex_2d) : sizeof(vertex_3d);
    glDrawElementsBaseVertex(GL_TRIANGLES, internal_data->index_count, GL_UNSIGNED_INT, (void*)(u64)internal_data->index_buffer_offset, internal_data->vertex_buffer_offset / stride);
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

void opengl_backend_create_geometry(struct renderer_backend* backend, geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices)
{
    u32 internal_id = state_ptr->next_internal_geometry_id++;
    geometry->internal_id = internal_id;
    u32 total_vertex_size = vertex_size * vertex_count;
    u32 total_index_size = index_size * index_count;
    opengl_geometry_data* internal_data = &state_ptr->geometries[internal_id];
    internal_data->vertex_count = vertex_count;
    internal_data->index_count = index_count;
    u64 vertex_offset = 0;
    if (!render_buffer_allocate(&state_ptr->object_vertex_buffer, total_vertex_size, &vertex_offset)) 
    {
        MERROR("opengl_backend_create_geometry failed to allocate vertex buffer space!");
        return;
    }

    internal_data->vertex_buffer_offset = vertex_offset;
    render_buffer_load_data(&state_ptr->object_vertex_buffer, internal_data->vertex_buffer_offset, total_vertex_size, vertices);
    if (index_count > 0) 
    {
        u64 index_offset = 0;
        if (!render_buffer_allocate(&state_ptr->object_index_buffer, total_index_size, &index_offset))
        {
            MERROR("opengl_backend_create_geometry failed to allocate index buffer space!");
            return;
        }

        internal_data->index_buffer_offset = index_offset;
        render_buffer_load_data(&state_ptr->object_index_buffer, internal_data->index_buffer_offset, total_index_size, indices);
    }
}

void opengl_backend_destroy_geometry(struct renderer_backend* backend, geometry* geometry)
{
    if (geometry && geometry->internal_id != 0) 
    {
        opengl_geometry_data* internal_data = &state_ptr->geometries[geometry->internal_id];
        u32 total_vertex_size = sizeof(vertex_3d) * internal_data->vertex_count;
        render_buffer_free(&state_ptr->object_vertex_buffer, total_vertex_size, internal_data->vertex_buffer_offset);
        if (internal_data->index_count > 0)
        {
            u32 total_index_size = sizeof(u32) * internal_data->index_count;
            render_buffer_free(&state_ptr->object_index_buffer, total_index_size, internal_data->index_buffer_offset);
        }

        Mzero_memory(internal_data, sizeof(opengl_geometry_data));
        geometry->internal_id = 0;
    }
}

b8 opengl_backend_begin_render_pass(struct renderer_backend* backend, u8 pass_id)
{
    switch (pass_id)
    {
    case BUILTIN_RENDER_PASS_WORLD:
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        break;

    case BUILTIN_RENDER_PASS_UI:
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;

    default:
        MERROR("opengl_backend_begin_render_pass called with unrecognized pass_id: %d", pass_id);
        return FALSE;
    }
    return TRUE;
}

void opengl_backend_end_render_pass(struct renderer_backend* backend, u8 pass_id)
{
    if (pass_id == BUILTIN_RENDER_PASS_UI)
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

b8 opengl_backend_shader_create(struct renderer_backend* backend, struct shader* shader, const struct shader_config* config) 
{
    opengl_shader* out_shader = (opengl_shader*)Mallocate(sizeof(opengl_shader), MEMORY_TAG_RENDERER);
    if (!opengl_shader_create(config->name, out_shader)) return FALSE;
    for (u32 i = 0; i < config->attribute_count; ++i) 
    {
        opengl_shader_add_attribute(out_shader, config->attributes[i].name, config->attributes[i].type);
    }
    for (u32 i = 0; i < config->uniform_count; ++i) 
    {
        opengl_shader_add_uniform(out_shader, config->uniforms[i].name, config->uniforms[i].type);
    }

    opengl_shader_initialize(out_shader);
    shader->internal_data = out_shader;
    return TRUE;
}

void opengl_backend_shader_destroy(struct renderer_backend* backend, struct shader* shader) 
{
    if (shader && shader->internal_data) 
    {
        opengl_shader_destroy((opengl_shader*)shader->internal_data);
        Mfree(shader->internal_data, sizeof(opengl_shader), MEMORY_TAG_RENDERER);
        shader->internal_data = 0;
    }
}

b8 opengl_backend_shader_use(struct renderer_backend* backend, struct shader* shader)
{
    return opengl_shader_use((opengl_shader*)shader->internal_data);
}

b8 opengl_backend_shader_set_uniform(struct renderer_backend* backend, struct shader* shader, u16 index, void* value) 
{
    if (shader && shader->internal_data)
    {
        return opengl_shader_set_uniform((opengl_shader*)shader->internal_data, index, value);
    }
    return FALSE;
}

void opengl_backend_update_global_matrices(struct renderer_backend* backend, mat4 projection, mat4 view)
{
    if (state_ptr)
    {
        global_uniform_data ubo_data = {};
        ubo_data.projection = projection;
        ubo_data.view = view;
        ubo_data.ambient_color = vect4_create(0.25f, 0.25f, 0.25f, 1.0f);
        camera* active_cam = camera_system_get_default();
        vect3 cam_pos = camera_position_get(active_cam);
        ubo_data.view_position = vect4_create(cam_pos.x, cam_pos.y, cam_pos.z, 1.0f);

        directional_light* dir = light_system_get_directional();
        if (dir) ubo_data.dir_light = *dir;

        ubo_data.num_p_lights = light_system_get_point_light_count();
        point_light* p_lights = light_system_get_point_lights();

        if (p_lights) 
        {
            for (u32 i = 0; i < ubo_data.num_p_lights; ++i)
            {
                ubo_data.p_lights[i] = p_lights[i];
            }
        }

        render_buffer_load_data(&state_ptr->global_ubo, 0, sizeof(global_uniform_data), &ubo_data);
    }
}