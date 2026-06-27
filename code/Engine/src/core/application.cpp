#include "application.h"
#include "game_types.h"
#include <Platform/Platform.h>	
#include "logger.h"
#include "M_memory.h"
#include "event.h"

typedef struct application_state
{
	game* game_inst;
	b8 is_run;
	b8 is_suspended;
	platform_state platform;
	i16 width;
	i16 height;
	f64 last_time;
};

static b8 initialized = FALSE;  //safety check
static application_state app_state;

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
	MINFO(get_memory_usage_str());

	while (app_state.is_run)
	{
		if (!platform_pump_messages(&app_state.platform))
		{
			app_state.is_run = FALSE;
		}

		if (!app_state.is_suspended)
		{
			if (!app_state.game_inst->update(app_state.game_inst, (f32)0))
			{
				MFATAL("Game Update failed, shutting down");
				app_state.is_run = FALSE;
				break;
			}

			if (!app_state.game_inst->render(app_state.game_inst, (f32)0))
			{
				MFATAL("Game Failed to render, shutting down");
				app_state.is_run = FALSE;
				break;
			}
		}
	}

	app_state.is_run = FALSE;

	event_shutdown();

	platform_shutdown(&app_state.platform);

	return TRUE;
}