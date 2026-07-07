#pragma once
#include "defines.hpp"

typedef struct opengl_image
{
	u32 handle;
	u32 width;
	u32 height;
	u32 format;
} opengl_image;

void opengl_image_create(u32 width, u32 height, u32 format, b32 create_view, opengl_image* out_image);

void opengl_image_view_create(u32 format, opengl_image* image);

void opengl_image_destroy(opengl_image* image);
