#include "event.h"
#include "M_memory.h"
#include <darray.h>

typedef struct registered_event
{
	void* listener;
	PFN_on_event callback;
}registered_event;

typedef struct event_code_entry
{
	registered_event* events;
}event_code_entry;

#define MAX_MESSAGE_CODES 10000

typedef struct event_system_state
{
	event_code_entry registered[MAX_MESSAGE_CODES];
}event_system_state;

//event system internal state
static b8 is_initialized = FALSE;
static event_system_state state;

// Initializes the event system. Returns TRUE if successful, FALSE otherwise.
b8 event_initialize()
{
	if (is_initialized == TRUE)
	{
		return FALSE;
	}
	is_initialized = FALSE;
	Mzero_memory(&state, sizeof(state));
	is_initialized = TRUE;
	return TRUE;
}

// Shuts down the event system and frees any allocated memory.
void event_shutdown()
{
	//free the events arrays
	for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i)
	{
		if(state.registered[i].events != 0)
		{
			darray_destroy(state.registered[i].events);
			state.registered[i].events = 0;
		}
	}
}

// Registers a listener for a specific event code. Returns TRUE if successful, FALSE otherwise.
b8 event_register(u16 code, void* listener, PFN_on_event on_event)
{
	if (is_initialized == FALSE)
	{
		return FALSE;
	}

	if(state.registered[code].events == 0)
	{
		state.registered[code].events = (registered_event*)darray_create(registered_event);
	}

	u64 registered_count = darray_length(state.registered[code].events);
	for (u64 i = 0; i < registered_count; ++i)
	{
		if(state.registered[code].events[i].listener == listener)
		{
			//todo: Warning
			return FALSE;
		}
	}

	registered_event event;
	event.listener = listener;
	event.callback = on_event;
	darray_push(state.registered[code].events, event);

	return TRUE;
}

// Unregisters a listener for a specific event code. Returns TRUE if the listener was successfully unregistered, FALSE otherwise.
b8 event_unregister(u16 code, void* listener, PFN_on_event on_event)
{
	if (is_initialized == FALSE)
	{
		return FALSE;
	}
	if(state.registered[code].events == 0)
	{
		//todo: WARNING
		return FALSE;
	}
	u64 registered_count = darray_length(state.registered[code].events);
	for (u64 i = 0; i < registered_count; ++i)
	{
		registered_event e = state.registered[code].events[i];
		if(e.listener == listener && e.callback == on_event)
		{
			registered_event popped_event;
			darray_pop_at(state.registered[code].events, i, &popped_event);
			return TRUE;
		}
	}
	return FALSE;
}

// Fires an event to all registered listeners for the given code. Returns TRUE if any listener handled the event, FALSE otherwise.
b8 event_fire(u16 code, void* sender, event_context context)
{
	if (is_initialized == FALSE)
	{
		return FALSE;
	}
	if(state.registered[code].events == 0)
	{
		return FALSE;
	}
	u64 registered_count = darray_length(state.registered[code].events);
	for (u64 i = 0; i < registered_count; ++i)
	{
		registered_event e = state.registered[code].events[i];
		if(e.callback(code, sender, e.listener, context))
		{
			return TRUE;
		}
	}
	return FALSE;
}