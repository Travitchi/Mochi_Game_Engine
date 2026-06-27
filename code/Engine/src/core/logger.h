#pragma once

#include "defines.hpp"

#define LOG_WARN_ENABLED 1;
#define LOG_DEBUG_ENABLED 1;
#define LOG_INFO_ENABLED 1;
#define LOG_TRACE_ENABLED 1;

// This is going to disable debug and trace logs in release builds, which is what we want
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0;
#define LOG_TRACE_ENABLED 0;
#endif

typedef enum log_level
{
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_TRACE = 5
} log_level;

b8 initialize_logging();  //can create a file where it logs messages, or just log to console for now
void shutdown_logging();

KAPI void log_output(log_level level, const char* message, ...); // where all of the logging output thunnels through, this will be the function that actually 

#define MFATAL(message,...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__) // it logs FATAl messages on the console

#ifndef MERROR
#define MERROR(message,...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__) // it logs ERROR messages on the console
#endif

#if LOG_WARN_ENABLED == 1
#define MWARN(message,...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__) // it logs WARN messages on the console
#else
#define MWARN(message,...) // does nothing
#endif

#if LOG_INFO_ENABLED == 1
#define MINFO(message,...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__) // it logs INFO messages on the console
#else
#define MINFO(message,...) // does nothing
#endif

#if LOG_DEBUG_ENABLED == 1
#define MDEBUG(message,...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__) // it logs DEBUG messages on the console
#else
#define MDEBUG(message,...) // does nothing
#endif

#if LOG_TRACE_ENABLED == 1
#define MTRACE(message,...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__) // it logs TRACE messages on the console
#else
#define MTRACE(message,...) // does nothing
#endif
