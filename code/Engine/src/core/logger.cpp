#include "logger.h"
#include "asserts.h"
#include "Platform/Platform.h"
//temporary
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


void report_assertion_fail(const char* expression, const char* message, const char* file, i32 line)
{
	log_output(LOG_LEVEL_FATAL, "Assertion Fail: %s , message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

b8 initialize_logging()
{
	//create log file
	return true;
}

void shutdown_logging()
{
	// cleanup log file
}

void log_output(log_level level, const char* message, ...)
{
	const char* level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
	b8 is_error = level < LOG_LEVEL_WARN;
	const i32 msg_lenght = 32000;
	char out_mess[msg_lenght];
	memset(out_mess, 0, sizeof(out_mess));

	va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(out_mess, msg_lenght, message, arg_ptr);
	va_end(arg_ptr);

	char out_mess2[msg_lenght];
	sprintf_s(out_mess2, "%s%s\n", level_strings[level], out_mess);
	
	if (is_error)
	{
		platform_console_write_error(out_mess2, level);
	}
	else
	{
		platform_console_write(out_mess2, level);
	}
}
