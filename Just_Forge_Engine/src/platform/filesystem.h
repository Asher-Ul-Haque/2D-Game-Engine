#pragma once
#include "defines.h"

typedef struct File 
{
    void* handle;
    bool isValid;
} File;

typedef enum fileModes
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2,
} fileModes;

FORGE_API bool fileExists(const char* PATH);

FORGE_API bool openFile(const char* PATH, fileModes MODE, bool BINARY_MODE, File* OUTPUT_FILE);

FORGE_API void closeFile(File* INPUT_FILE);

FORGE_API bool readFileLine(File* INPUT_FILE, char** BUFFER);

FORGE_API bool writeFileLine(File* INPUT_FILE, const char* TEXT);

FORGE_API bool readFile(File* INPUT_FILE, unsigned long long DATA_SIZE, void* BUFFER, unsigned long long* READ_BYTES);

FORGE_API bool readAllBytes(File* INPUT_FILE, unsigned char** OUTPUT, unsigned long long* READ_BYTES);

FORGE_API bool writeFile(File* INPUT_FILE, unsigned long long DATA_SIZE, const void* DATA, unsigned long long* WRITE_BYTES);
