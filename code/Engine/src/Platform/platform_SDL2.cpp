#include "Platform.h"
#if KPLATFORM_WINDOWS 
#include "core/logger.h"
#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

// SDL2 Platform layer
typedef struct internal_state {
    SDL_Window* window;
} internal_state;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

b8 platform_initialize(platform_state* plat_state, const char* application_name, i32 x, i32 y, i32 width, i32 height) 
{
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)plat_state->internal_state;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        MFATAL("Startup Failed: %s", SDL_GetError());
        return FALSE;
    }

    state->window = SDL_CreateWindow(application_name,x, y,width, height,SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (state->window == NULL) 
    {
        MFATAL("Creazione finestra fallita: %s", SDL_GetError());
        return FALSE;
    }
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	clock_frequency = 1.0 / (f64)frequency.QuadPart;
	QueryPerformanceCounter(&start_time);

    return TRUE;
}

void platform_shutdown(platform_state* plat_state) 
{
    if (plat_state && plat_state->internal_state) 
    {
        internal_state* state = (internal_state*)plat_state->internal_state;
        if (state->window) 
        {
            SDL_DestroyWindow(state->window);
        }
        free(state);
        plat_state->internal_state = 0;
    }
    SDL_Quit();
}

b8 platform_pump_messages(platform_state* plat_state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        switch (event.type) 
        {
        case SDL_QUIT:
            return FALSE;

        case SDL_KEYDOWN:
        case SDL_KEYUP: 
        {
            b8 pressed = (event.type == SDL_KEYDOWN);
            // SDL_Keycode key_code = event.key.keysym.sym;
            // TODO: Pass this to your Input System
        } break;

        case SDL_MOUSEMOTION: 
        {
            i32 x = event.motion.x;
            i32 y = event.motion.y;
            // TODO: Pass this to your Input System
        } break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: 
        {
            b8 pressed = (event.type == SDL_MOUSEBUTTONDOWN);

            // event.button.button tells you WHICH button it was:
            // SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT, SDL_BUTTON_MIDDLE
            u8 mouse_button = event.button.button;

            // TODO: Pass this to your Input System
        } break;

        case SDL_MOUSEWHEEL: 
        {
            // event.wheel.y is positive if scrolling up, negative if scrolling down
            i32 z_delta = event.wheel.y;

            // TODO: Pass this to your Input System
        } break;

        case SDL_WINDOWEVENT: 
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) 
            {
                // i32 new_width = event.window.data1;
                // i32 new_height = event.window.data2;
                // TODO: Fire an event for window resize
            }
        } break;
        }
    }
    return TRUE;
}

// Memory management functions
void* platform_allocate(u64 size, b8 aligned) 
{
    return malloc(size);
}
void platform_free(void* block, b8 aligned)
{ free(block);
}
void* platform_zero_memory(void* block, u64 size)
{ 
    return memset(block, 0, size);
}
void* platform_copy_memory(void* dest, const void* src, u64 size) 
{
    return memcpy(dest, src, size); 
}
void* platform_set_memory(void* dest, i32 value, u64 size) 
{ 
    return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 colour) 
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	static u8 levels[6] = { 64, 4, 6, 2, 1 ,8 }; // Fatal, Error, Warning, Info, Debug, Trace
	SetConsoleTextAttribute(console_handle, levels[colour]);
	OutputDebugStringA(message);
	u64 length = strlen(message);
    LPDWORD number_written = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 colour)
{
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[6] = { 64, 4, 6, 2, 1 ,8 }; // Fatal, Error, Warning, Info, Debug, Trace
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

// Timing functions
f64 platform_get_absolute_time() 
{
    LARGE_INTEGER now_time;
	QueryPerformanceCounter(&now_time);
	return (f64)now_time.QuadPart * clock_frequency;
}
void platform_sleep(u64 ms) 
{
    Sleep(ms);
}

#endif