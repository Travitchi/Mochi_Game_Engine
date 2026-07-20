#include "logger.h"
#include "asserts.h"
#include "Platform/Platform.h"
#include <stdarg.h>
#include <fstream>


static std::ofstream log_file;

void report_assertion_fail(const char* expression, const char* message, const char* file, i32 line)
{
	log_output(LOG_LEVEL_FATAL, "Assertion Fail: %s , message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

b8 initialize_logging()
{
	log_file.open("console.log", std::ios::out | std::ios::trunc);
	if (!log_file.is_open())
	{
		platform_console_write_error("Failed to open console.log for writing!\n", LOG_LEVEL_ERROR);
		return FALSE;
	}

	return TRUE;
}

void shutdown_logging()
{
	if (log_file.is_open())
	{
		log_file.close();
	}
}

void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };
    b8 is_error = level < LOG_LEVEL_WARN;

    va_list arg_ptr;
    va_start(arg_ptr, message);
    int length = vsnprintf(nullptr, 0, message, arg_ptr);
    va_end(arg_ptr);

    std::string formatted_message(length, '\0');
    va_start(arg_ptr, message);
    vsnprintf(&formatted_message[0], length + 1, message, arg_ptr);
    va_end(arg_ptr);

    std::string final_output = std::string(level_strings[level]) + formatted_message + "\n";

    static std::string last_log_message = "";
    if (final_output == last_log_message)
    {
        return;
    }
    last_log_message = final_output;

    if (is_error) {
        platform_console_write_error(final_output.c_str(), level);
    }
    else {
        platform_console_write(final_output.c_str(), level);
    }

    if (log_file.is_open()) {
        log_file << final_output;
        log_file.flush();
    }
}
