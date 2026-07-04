#include "application.h"
#include "game_types.h"
#include <Platform/Platform.h>	
#include "logger.h"
#include "M_memory.h"
#include "event.h"
#include "input.h"
#include "SDL.h"
#include "clock.h"
#include "renderer_frontend.h"

typedef struct application_state
{
	game* game_inst;
	b8 is_run;
	b8 is_suspended;
	platform_state platform;
	i16 width;
	i16 height;
	clock clock;
	f64 last_time;
};

static b8 initialized = FALSE;  //safety check
static application_state app_state;

//event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_resize(u16 code, void* sender, void* listener_inst, event_context context);


KAPI b8 application_create(game* game_inst)
{
	if (initialized)
	{
		MERROR("application_create called more than once. ");
		return FALSE;
	}

	app_state.game_inst = game_inst;

	//initialize subsystem
	initialize_logging();
	input_initialize();

	//todo: remove this
	MFATAL("This is a fatal message with a number: %d", 42);
	MERROR("This is an error message with a string: %s", "Hello, World!");
	MWARN("This is a warning message.");
	MINFO("This is an info message.");
	MDEBUG("This is a debug message.");
	MTRACE("This is a trace message.");

	app_state.is_run = TRUE;
	app_state.is_suspended = FALSE;


	if (!event_initialize())
	{
		MFATAL("Event System Failed to initialize!!");
		return FALSE;
	}


	event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
	event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
	event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
	event_register(EVENT_CODE_RESIZED, 0, application_on_resize);

	if (!platform_initialize(
		&app_state.platform,
		game_inst->app_config.name,
		game_inst->app_config.start_pos_x,
		game_inst->app_config.start_pos_y,
		game_inst->app_config.start_width,
		game_inst->app_config.start_height)
		)
	{
		return FALSE;
	}

	//renderer startup
	if (!renderer_initialize(game_inst->app_config.name, &app_state.platform)) 
	{
		MFATAL("Failed to initialize renderer. Shutting down.");
		return FALSE;
	}

	app_state.width = game_inst->app_config.start_width;
	app_state.height = game_inst->app_config.start_height;
	renderer_on_resize(app_state.width, app_state.height);

	//initialize game
	if (!app_state.game_inst->initialize(app_state.game_inst))
	{
		MFATAL("Game Failed to Initialize");
		return FALSE;
	}

	app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

	initialized = TRUE;
	return TRUE;
}

KAPI b8 application_run()
{
	clock_start(&app_state.clock);
	clock_update(&app_state.clock);
	app_state.last_time = app_state.clock.elapsed_time;
	f64 running_time = 0;
	u8 frame_count = 0;
	f64 target_frame_seconds = 1.0f / 60;

	MINFO(get_memory_usage_str());

	while (app_state.is_run)
	{
		
		if (!platform_pump_messages(&app_state.platform))
		{
			app_state.is_run = FALSE;
		}

		if (!app_state.is_suspended)
		{
			clock_update(&app_state.clock);
			f64 current_time = app_state.clock.elapsed_time;
			f64 delta = (current_time - app_state.last_time);
			f64 frame_start_time = platform_get_absolute_time();
			if (!app_state.game_inst->update(app_state.game_inst, (f32)delta))
			{
				MFATAL("Game Update failed, shutting down");
				app_state.is_run = FALSE;
				break;
			}

			if (!app_state.game_inst->render(app_state.game_inst, (f32)delta))
			{
				MFATAL("Game Failed to render, shutting down");
				app_state.is_run = FALSE;
				break;
			}

			//todo: refactor packet creation
			render_packet packet;
			packet.delta_time = delta;
			renderer_draw_frame(&packet);

			f64 frame_end_time = platform_get_absolute_time();
			f64 frame_elapsed_time = frame_end_time - frame_start_time;
			running_time += frame_elapsed_time;
			f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

			if (remaining_seconds > 0)
			{
				u64 remaining_ms = (remaining_seconds * 1000);
				b8 limit_frames = FALSE;
				if (remaining_ms > 0 && limit_frames)
				{
					platform_sleep(remaining_ms - 1);
				}

				frame_count++;
			}


			input_update(delta);
			app_state.last_time = current_time;


		}
	}

	app_state.is_run = FALSE;

	//shutdown system
	event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
	event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
	event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
	event_shutdown();
	input_shutdown();
	renderer_shutdown();

	platform_shutdown(&app_state.platform);
	shutdown_logging();

	return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context)
{
	switch (code)
	{
	case EVENT_CODE_APPLICATION_QUIT: 
		{
		MINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down. \n");
		app_state.is_run = FALSE;
		return TRUE;
		}
	}

	return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) 
{
	if (code == EVENT_CODE_KEY_PRESSED) 
	{
		u16 key_code = context.data.u16[0];

		if (key_code == SDL_SCANCODE_ESCAPE) 
		{
			event_context data = {};
			event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);

			return TRUE;
		}
	}
	return FALSE;
}

b8 application_on_resize(u16 code, void* sender, void* listener_inst, event_context context)
{
	if (code == EVENT_CODE_RESIZED)
	{
		u16 width = context.data.u16[0];
		u16 height = context.data.u16[1];
		if (width != app_state.width || height != app_state.height)
		{
			app_state.width = width;
			app_state.height = height;
			MINFO("Window resized to: %i x %i", width, height);
			renderer_on_resize(width, height);
			app_state.game_inst->on_resize(app_state.game_inst, width, height);
		}
		return TRUE;
	}
	return FALSE;
}
