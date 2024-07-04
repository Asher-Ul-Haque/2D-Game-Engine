#pragma once
#include "defines.h"


// - - - | Log Macros and Defs | - - -


// - - - Log Enables - - -

// - - - Handles
//No handles for fatal and error: always enabled
#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

//Disable and debug, and trace for release builds
#if FORGE_RELEASE == 1
    #define LOG_WARNING_ENABLED 0 
    #define LOG_INFO_ENABLED 0
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif


// - - -  Log Levels  - - -

typedef enum LogLevel 
{
    LOG_LEVEL_FATAL = 0, //Just give up and die 
    LOG_LEVEL_ERROR = 1, //Something went wrong badly
    LOG_LEVEL_WARNING = 2, //Are you sure about that
    LOG_LEVEL_INFO = 3, //Just some information
    LOG_LEVEL_DEBUG = 4, //Debugging information
    LOG_LEVEL_TRACE = 5 //Trace every step
} LogLevel;


// - - - | Log Functions | - - -


// - - - System Controls - - -

bool initializeLogger(unsigned long long* MEMORY_REQUIREMENT, void* STATE);
void shutdownLogger(void* STATE);


// - - - API Controls - - -

FORGE_API void logOutput(LogLevel LEVEL, const char* MESSAGE, ...); //Multivariate, takes any number of arguments greater than 1

// - - - Fatal log
//Always define FATAL and ERROR logs.
#ifndef FORGE_LOG_FATAL
#define FORGE_LOG_FATAL(MESSAGE, ...) logOutput(LOG_LEVEL_FATAL, MESSAGE, ##__VA_ARGS__);
#endif

#ifndef FORGE_LOG_ERROR
#define FORGE_LOG_ERROR(MESSAGE, ...) logOutput(LOG_LEVEL_ERROR, MESSAGE, ##__VA_ARGS__);
#endif

//For the rest, define only when enabled, else define to nothingness
#if LOG_WARNING_ENABLED == 1
#define FORGE_LOG_WARNING(MESSAGE, ...) logOutput(LOG_LEVEL_WARNING, MESSAGE, ##__VA_ARGS__);
#else
#define FORGE_LOG_WARNING(MESSAGE, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define FORGE_LOG_INFO(MESSAGE, ...) logOutput(LOG_LEVEL_INFO, MESSAGE, ##__VA_ARGS__);
#else
#define FORGE_LOG_INFO(MESSAGE, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define FORGE_LOG_DEBUG(MESSAGE, ...) logOutput(LOG_LEVEL_DEBUG, MESSAGE, ##__VA_ARGS__);
#else
#define FORGE_LOG_DEBUG(MESSAGE, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define FORGE_LOG_TRACE(MESSAGE, ...) logOutput(LOG_LEVEL_TRACE, MESSAGE, ##__VA_ARGS__);
#else
#define FORGE_LOG_TRACE(MESSAGE, ...)
#endif
