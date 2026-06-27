#pragma once
#include "defines.hpp"
// turn on and off assertions 
#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debug_break() __debugbreak()
#else
#define debug_break() __builtin_trap()
#endif

KAPI void report_assertion_fail(const char* expression, const char* message, const char* file, i32 line);

#define MASSERT(expr)													\
	{																	\
		if (expr) {}													\
		else															\
		{																\
			report_assertion_fail(#expr, "", __FILE__, __LINE__);		\
			debug_break();												\
		}																\
	}

#define MASSERT_MSG (expr, message)										\
	{																	\
		if (expr) {}													\
		else															\
		{																\
			report_assertion_fail(#expr, message, __FILE__, __LINE__);  \
			debug_break();												\
		}																\
	}

#ifdef _DEBUG
#define MASSERT_DEBUG(expr)												\
	{																	\
		if (expr) {}													\
		else															\
		{																\
			report_assertion_fail(#expr, "", __FILE__, __LINE__);		\
			debug_break();												\
		}																\
	}
#else
#define MASSERT_DEBUG(expr)
#endif

#else 
#define MASSERT(expr)
#define MASSERT_MSG(expr, message)
#define MASSERT_DEBUG(expr)
#endif