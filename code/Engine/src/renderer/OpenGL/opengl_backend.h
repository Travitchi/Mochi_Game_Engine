#pragma once
#include "renderer_backend.h"

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
void opengl_backend_shutdown(renderer_backend* backend);
void opengl_backend_resized(renderer_backend* backend, u16 width, u16 height);
b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time);
b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time);