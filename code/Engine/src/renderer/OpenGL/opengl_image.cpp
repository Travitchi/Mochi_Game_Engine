#include "opengl_image.h"
#include "logger.h"
#include "M_memory.h"
#include "glad.h"

void opengl_image_create(u32 width, u32 height, u32 format, b32 create_view, opengl_image* out_image)
{
    if (!out_image)
    {
        MERROR("opengl_image_create requires a valid out_image pointer.");
        return;
    }

    Mzero_memory(out_image, sizeof(opengl_image));
    out_image->width = width;
    out_image->height = height;
    out_image->format = format;

    // Generates a new texture ID on the GPU
    glGenTextures(1, &out_image->handle);
    if (out_image->handle == 0)
    {
        MFATAL("Failed to generate OpenGL texture handle!");
        return;
    }

    glBindTexture(GL_TEXTURE_2D, out_image->handle);

    u32 ext_format = GL_RGBA;
    u32 data_type = GL_UNSIGNED_BYTE;

    if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_COMPONENT24 || format == GL_DEPTH24_STENCIL8)
    {
        // Shadow map / Depth texture configuration
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        f32 border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ext_format = GL_DEPTH_COMPONENT;
        data_type = GL_FLOAT;
    }
    else
    {
        // Standard color texture configuration
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (format == GL_RGB)
        {
            ext_format = GL_RGB;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, ext_format, data_type, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (create_view)
    {
        opengl_image_view_create(format, out_image);
    }

    MINFO("Successfully created OpenGL image (ID: %u, Width: %u, Height: %u).", out_image->handle, width, height);
}

void opengl_image_view_create(u32 format, opengl_image* image)
{
    if (!image || image->handle == 0) 
    {
        MERROR("opengl_image_view_create called with an invalid image!");
        return;
    }
    image->format = format;
}

void opengl_image_destroy(opengl_image* image)
{
    if (image && image->handle != 0) 
    {
        MINFO("Destroying OpenGL image (ID: %u).", image->handle);
        glDeleteTextures(1, &image->handle);
        Mzero_memory(image, sizeof(opengl_image));
    }
}