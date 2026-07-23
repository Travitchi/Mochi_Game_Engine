#include "application.h"
#include "game_types.h"
#include "Platform.h"
#include "logger.h"
#include "M_memory.h"
#include "event.h"
#include "input.h"
#include "SDL.h"
#include "clock.h"
#include "renderer_frontend.h"
#include "texture_systems.h"
#include "material_systems.h"
#include "geometry_systems.h"
#include "resource_systems.h"
#include "camera_system.h"
#include "light_system.h"
#include "image_loader.h"
#include "text_loader.h"
#include "material_loader.h"
#include "shader_system.h"
#include "shader_loader.h"
#include <string.h>

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
	u64 texture_system_memory_requirement;
	void* texture_system_state_memory;
	u64 material_system_memory_requirement;
	void* material_system_state_memory;
	u64 resource_system_memory_requirement;
	void* resource_system_state_memory;
	u64 geometry_system_memory_requirement;
	void* geometry_system_state_memory;
	void* shader_system_state_memory;
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

	game_inst->state = Mallocate(game_inst->state_memory_requirement, MEMORY_TAG_GAME);
	if (!game_inst->state)
	{
		MFATAL("Failed to allocate game state!");
		return FALSE;
	}


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

	//resource system startup
	resource_system_config res_config = {};
	res_config.max_loader_count = 32;
	res_config.asset_base_path = "../../Assets";
	u64 res_sys_memory_req = 0;
	resource_system_initialize(&res_sys_memory_req, 0, res_config);
	app_state.resource_system_state_memory = Mallocate(res_sys_memory_req, MEMORY_TAG_APPLICATION);
	resource_system_initialize(&res_sys_memory_req, app_state.resource_system_state_memory, res_config);
	resource_system_register_loader(image_resource_loader_create());
	resource_system_register_loader(text_resource_loader_create());
	resource_system_register_loader(material_resource_loader_create());
	resource_system_register_loader(shader_resource_loader_create());

	//renderer startup
	if (!renderer_initialize(game_inst->app_config.name, &app_state.platform)) 
	{
		MFATAL("Failed to initialize renderer. Shutting down.");
		return FALSE;
	}

	//camera system startup
	camera_system_config cam_sys_config = {};
	cam_sys_config.max_camera_count = 61;
	u64 camera_sys_memory_req = 0;
	camera_system_initialize(&camera_sys_memory_req, 0, cam_sys_config);
	void* camera_sys_state = Mallocate(camera_sys_memory_req, MEMORY_TAG_APPLICATION);
	if (!camera_system_initialize(&camera_sys_memory_req, camera_sys_state, cam_sys_config))
	{
		MFATAL("Failed to initialize camera system.");
		return FALSE;
	}

	//light system startup
	if (!light_system_initialize())
	{
		MFATAL("Failed to initialize light system.");
		return FALSE;
	}
	//test lights
	light_system_add_point_light(vect4_create(0.0f, 1.0f, 0.0f, 1.0f), vect3_create(15.0f, 5.0f, -15.0f), 1.0f, 0.09f, 0.032f);
	light_system_add_point_light(vect4_create(5.0f, 0.0f, 0.0f, 1.0f), vect3_create(-15.0f, 5.0f, -15.0f), 1.0f, 0.09f, 0.032f);

	//texture system startup
	texture_system_config tex_config;
	tex_config.max_texture_count = 65536;
	texture_system_initialize(&app_state.texture_system_memory_requirement, 0, tex_config);
	app_state.texture_system_state_memory = Mallocate(app_state.texture_system_memory_requirement, MEMORY_TAG_RENDERER);
	texture_system_initialize(&app_state.texture_system_memory_requirement, app_state.texture_system_state_memory, tex_config);

	//shader system startup
	shader_system_config shader_config;
	shader_config.max_shader_count = 1024;
	u64 shader_sys_memory_req = 0;
	shader_system_initialize(&shader_sys_memory_req, 0, shader_config);
	void* shader_sys_state = Mallocate(shader_sys_memory_req, MEMORY_TAG_RENDERER);
	app_state.shader_system_state_memory = Mallocate(shader_sys_memory_req, MEMORY_TAG_RENDERER);
	shader_system_initialize(&shader_sys_memory_req, shader_sys_state, shader_config);
	resource obj_shader_res;
	if (resource_system_load("M_object_shader", RESOURCE_TYPE_SHADER, &obj_shader_res)) 
	{
		shader out_s;
		shader_system_create_shader((const struct shader_config*)obj_shader_res.data, &out_s);
		resource_system_unload(&obj_shader_res);
	}

	resource ui_shader_res;
	if (resource_system_load("M_ui_shader", RESOURCE_TYPE_SHADER, &ui_shader_res))
	{
		shader out_s;
		shader_system_create_shader((const struct shader_config*)ui_shader_res.data, &out_s);
		resource_system_unload(&ui_shader_res);
	}

	//material system startup
	material_system_config mat_config;
	mat_config.max_material_count = 4096;
	u64 mat_sys_memory_req = 0;
	material_system_initialize(&mat_sys_memory_req, 0, mat_config);
	void* mat_sys_state = Mallocate(mat_sys_memory_req, MEMORY_TAG_RENDERER);
	material_system_initialize(&mat_sys_memory_req, mat_sys_state, mat_config);

	//geometry system startup
	geometry_system_config geo_config = { 4096 };
	u64 geo_sys_mem_req = 0;
	geometry_system_initialize(&geo_sys_mem_req, 0, geo_config);
	void* geo_sys_state = Mallocate(geo_sys_mem_req, MEMORY_TAG_APPLICATION);
	geometry_system_initialize(&geo_sys_mem_req, geo_sys_state, geo_config);

	//set initial window size
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


	mat4 ui_projection = mat4_orthographic(0.0f, (f32)app_state.width, (f32)app_state.height, 0.0f, -100.0f, 100.0f);
	mat4 ui_view = mat4_id();


	vertex_2d ui_verts[4] = {
		{ {0.0f,   0.0f},   {0.0f, 1.0f} },
		{ {512.0f, 0.0f},   {1.0f, 1.0f} },
		{ {512.0f, 512.0f}, {1.0f, 0.0f} },
		{ {0.0f,   512.0f}, {0.0f, 0.0f} }
	};
	u32 ui_indices[6] = { 2, 1, 0, 3, 2, 0 };

	geometry_config ui_config = {};
	ui_config.vertex_size = sizeof(vertex_2d);
	ui_config.vertex_count = 4;
	ui_config.vertices = ui_verts;
	ui_config.index_size = sizeof(u32);
	ui_config.index_count = 6;
	ui_config.indices = ui_indices;
	strcpy_s(ui_config.name, 256, "test_ui_geom");
	strcpy_s(ui_config.material_name, 256, "test_ui_material");

	geometry_render_data my_ui_image = {};
	my_ui_image.geometry = geometry_system_acquire_from_config(ui_config, TRUE);
	my_ui_image.model = mat4_translation(vect3_create(10.0f, 10.0f, 0.0f));
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
			render_packet packet = {};
			packet.delta_time = delta;
			packet.geometry_count = 0;
			packet.ui_geometry_count = 0;

			vertex_2d ui_verts[4] = {{ {0.0f,   0.0f},   {0.0f, 1.0f} },{ {100.0f, 0.0f},   {1.0f, 1.0f} },{ {100.0f, 100.0f}, {1.0f, 0.0f} },{ {0.0f,   100.0f}, {0.0f, 0.0f} }};
			u32 ui_indices[6] = { 2, 1, 0, 3, 2, 0 };

			if (renderer_begin_frame(&packet))
			{
				if (renderer_begin_render_pass(BUILTIN_RENDER_PASS_WORLD))
				{
					renderer_push_world_matrices();
					renderer_draw_test_geometry();
					renderer_end_render_pass(BUILTIN_RENDER_PASS_WORLD);
				}
				/*
				if (renderer_begin_render_pass(BUILTIN_RENDER_PASS_UI))
				{
					renderer_update_global_matrices(ui_projection, ui_view);
					shader* ui_shader = shader_system_get("M_ui_shader");
					shader_system_use("M_ui_shader");
					mat4 ui_model = mat4_id();
					shader_system_set_uniform(ui_shader, "u_push.model", &ui_model);
					renderer_draw_geometry(my_ui_image);
					renderer_end_render_pass(BUILTIN_RENDER_PASS_UI);
				}
				*/
				renderer_end_frame(&packet);
			}

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
	light_system_shutdown();
	geometry_system_shutdown(app_state.geometry_system_state_memory);
	material_system_shutdown(app_state.material_system_state_memory);
	texture_system_shutdown(app_state.texture_system_state_memory);
	shader_system_shutdown(app_state.shader_system_state_memory);
	resource_system_shutdown(app_state.resource_system_state_memory);
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
