#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"
#include "platform/filesystem.h"


// - - - | Log Functions | - - -


// - - - System Controls - - -

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct loggerSystemState
{
    File logFile;
} loggerSystemState;

static loggerSystemState* statePtr;

void appendLogFile(const char* MESSAGE)
{
    if (statePtr && statePtr->logFile.isValid)
    {
        unsigned long long length = strlen(MESSAGE);
        unsigned long long written = 0;
        if (!writeFile(&statePtr->logFile, length, MESSAGE, &written))
        {
            platformWriteConsoleError("Failed to write to log file", LOG_LEVEL_ERROR);
        }
    }
}


bool initializeLogger(unsigned long long* MEMORY_REQUIREMENT, void* STATE)
{
    *MEMORY_REQUIREMENT = sizeof(loggerSystemState);
    if (STATE == 0)
    {
        return true;
    }

    statePtr = STATE;

    if (!openFile("forge.log", FILE_MODE_WRITE, false, &statePtr->logFile))
    {
        platformWriteConsoleError("Failed to open log file", LOG_LEVEL_FATAL);
        return false;
    }

    FORGE_LOG_INFO("Logging System Initialized");
    return true;
}

void shutdownLogger(void* STATE)
{
    FORGE_LOG_INFO("Logging System Shutdown");
    // TODO: cleanup and write all queued entries
    statePtr = 0;
}


// - - - API Controls - - -

void logOutput(LogLevel LEVEL, const char* MESSAGE, ...)
{
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    bool isError = LEVEL < LOG_LEVEL_WARNING;
    const int messageLength = 32000;
    char outputMessage[messageLength];
    memset(outputMessage, 0, sizeof(outputMessage));

    // Add the rest of the arguments
    __builtin_va_list argumentPointer;
    va_start(argumentPointer, MESSAGE);
    vsnprintf(outputMessage, sizeof(outputMessage), MESSAGE, argumentPointer);
    va_end(argumentPointer);

    //Prepend with level header
    char finalMessage[32000];
    sprintf(finalMessage, "%s%s\n", levelStrings[LEVEL], outputMessage);

    //write to the console
    if (isError)
    {
        platformWriteConsoleError(finalMessage, LEVEL);
    }
    else 
    {
        platformWriteConsole(finalMessage, LEVEL);    
    }

    //Write to the log file
    appendLogFile(finalMessage);
}


// - - - | Assert Functions | - - -


void reportAssertionFailure(const char* EXPRESSION, const char* MESSAGE, const char* FILE, int LINE)
{
    logOutput(LOG_LEVEL_FATAL, "Assertion Failed: %s, message: %s in file: %s at line: %d", EXPRESSION, MESSAGE, FILE, LINE);
}
