#include "input.h"
#include "event.h"
#include "M_memory.h"
#include "logger.h"

typedef struct keyboard_state
{
	b8 keys[256];
}keyboard_state;

typedef struct mouse_state
{
	i16 x;
	i16 y;
	u8 buttons[BUTTON_MAX_BUTTONS];
}mouse_state;

typedef struct input_state
{
	keyboard_state keyboard_current;
	keyboard_state keyboard_previous;
	mouse_state mouse_current;
	mouse_state mouse_previous;
}input_state;

static b8 initialized = FALSE;
static input_state state = {};
