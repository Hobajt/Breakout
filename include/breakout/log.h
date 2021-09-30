#pragma once

#include <stdio.h>
#include <exception>

#define ENABLE_ASSERTS
#define ENABLE_LOGGING
#define GL_DEBUG

#define BIT(n) (1<<(n))

#define LOG_GL		BIT(0)
#define LOG_ERROR	BIT(1)
#define LOG_WARN	BIT(2)
#define LOG_INFO	BIT(3)
#define LOG_DEBUG	BIT(4)
#define LOG_TRACE	BIT(5)
#define LOG_FINE	BIT(6)

#define LOG_CTOR	BIT(7)
#define LOG_DTOR	BIT(8)

#define LOG_LEVELS (unsigned int)(-1)

#ifdef ENABLE_LOGGING
#define LOG(level, ...) if((level) & LOG_LEVELS) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(level, ...)
#endif

#ifdef ENABLE_ASSERTS
#define ASSERT(condition) { if(!(condition)) { LOG(LOG_ERROR, "Assertion failed at %s:%d (%s).\n", __FILE__, __LINE__, #condition); throw std::exception(); } }
#define ASSERT_MSG(condition, ...) { if(!(condition)) { LOG(LOG_ERROR, "Assertion failed at %s:%d (%s).\n", __FILE__, __LINE__, #condition); LOG(LOG_ERROR, __VA_ARGS__); throw std::exception(); } }
#else
#define ASSERT(condition)
#define ASSERT_MSG(condition, ...)
#endif
