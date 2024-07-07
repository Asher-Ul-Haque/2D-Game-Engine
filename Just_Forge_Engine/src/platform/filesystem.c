#include "filesystem.h"

#include "core/logger.h"
#include "core/memory.h"

// TODO: own filesyste,
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

bool fileExists(const char* PATH)
{
    struct stat buffer;
    return stat(PATH, &buffer) == 0;
}

bool openFile(const char* PATH, fileModes MODE, bool BINARY_MODE, File* OUTPUT_FILE)
{
    OUTPUT_FILE->handle = 0;
    OUTPUT_FILE->isValid = false;
    
    const char* modeStr;
    if ((MODE & FILE_MODE_READ) != 0 && (MODE & FILE_MODE_WRITE) != 0)
    {
        modeStr = BINARY_MODE ? "w+b" : "w+";
    }
    else if ((MODE & FILE_MODE_READ) != 0 && (MODE & FILE_MODE_WRITE) == 0)
    {
        modeStr = BINARY_MODE ? "rb" : "r";
    }
    else if ((MODE & FILE_MODE_WRITE) != 0 && (MODE & FILE_MODE_READ) == 0)
    {
        modeStr = BINARY_MODE ? "wb" : "w";
    }
    else
    {
        FORGE_LOG_ERROR("Invalid file mode passeed while trying to open file: %s", PATH);
        return false;
    }

    //Attempt to open file
    FILE* file = fopen(PATH, modeStr);
    if (!file)
    {
        FORGE_LOG_ERROR("Failed to open file: %s", PATH);
        return false;
    }

    OUTPUT_FILE->handle = file;
    OUTPUT_FILE->isValid = true;

    return true;
}

void closeFile(File* INPUT_FILE)
{
    if (INPUT_FILE->isValid)
    {
        fclose((FILE*)INPUT_FILE->handle);
        INPUT_FILE->handle = 0;
        INPUT_FILE->isValid = false;
    }
    else
    {
        FORGE_LOG_WARNING("Attempted to close an invalid file");
    }
}

bool readFileLine(File* INPUT_FILE, char** BUFFER)
{
    if (INPUT_FILE->handle)
    {
        // Assumption. since we are reading a single line, assume to assume that 32000 is enough characters
        char buffer[32000];
        if (fgets(buffer, 32000, (FILE*)INPUT_FILE->handle) != 0)
        {
            unsigned long long length = strlen(buffer);
            *BUFFER = forgeAllocateMemory((sizeof(char) * length) + 1, MEMORY_TAG_FILE);
            strcpy(*BUFFER, buffer);
            return true;
        }
        FORGE_LOG_WARNING("Failed to read line from file");
    }
    FORGE_LOG_WARNING("Attempted to read from an invalid file");
    return false;
}

bool writeFileLine(File* INPUT_FILE, const char* TEXT)
{
    if (INPUT_FILE->handle)
    {
        int result = fputs(TEXT, (FILE*)INPUT_FILE->handle);
        if (result != EOF)
        {
            result = fputc('\n', (FILE*)INPUT_FILE->handle);
        }

        // Make sure to flush the stream so it is written to the file immediately
        // This prevents data loss in case of crash
        fflush((FILE*)INPUT_FILE->handle);
        return result != EOF;
    }
    FORGE_LOG_WARNING("Attempted to write to an invalid file");
    return false;
}

bool readFile(File* INPUT_FILE, unsigned long long DATA_SIZE, void* BUFFER, unsigned long long* READ_BYTES)
{
    if (INPUT_FILE->handle && READ_BYTES)
    {
        *READ_BYTES = fread(BUFFER, 1, DATA_SIZE, (FILE*)INPUT_FILE->handle);
        if (*READ_BYTES != DATA_SIZE)
        {
            FORGE_LOG_WARNING("Failed to read %llu bytes from file", DATA_SIZE);
            return false;
        }
        return true;
    }
    FORGE_LOG_WARNING("Attempted to read from an invalid file");
    return false;
}

bool readAllBytes(File* INPUT_FILE, unsigned char** OUTPUT, unsigned long long* READ_BYTES)
{
    if (INPUT_FILE->handle)
    {
        //File size
        fseek((FILE*)INPUT_FILE->handle, 0, SEEK_END);
        unsigned long long size = ftell((FILE*)INPUT_FILE->handle);
        rewind((FILE*)INPUT_FILE->handle);

        *OUTPUT = forgeAllocateMemory(size, MEMORY_TAG_FILE);
        *READ_BYTES = fread(*OUTPUT, 1, size, (FILE*)INPUT_FILE->handle);

        if (*READ_BYTES != size)
        {
            FORGE_LOG_WARNING("Failed to read %llu bytes from file", size);
            return false;
        }
        return true;
    }
    FORGE_LOG_WARNING("Attempted to read from an invalid file");
    return false;
}

bool writeFile(File* INPUT_FILE, unsigned long long DATA_SIZE, const void* DATA, unsigned long long* WRITE_BYTES)
{
    if (INPUT_FILE->handle)
    {
        *WRITE_BYTES = fwrite(DATA, 1, DATA_SIZE, (FILE*)INPUT_FILE->handle);
        if (*WRITE_BYTES != DATA_SIZE)
        {
            FORGE_LOG_WARNING("Failed to write %llu bytes to file", DATA_SIZE);
            return false;
        }

        // Make sure to flush the stream so it is written to the file immediately
        // This prevents data loss in case of crash
        fflush((FILE*)INPUT_FILE->handle);
        return true;
    }
    FORGE_LOG_WARNING("Attempted to write to an invalid file");
    return false;
}
