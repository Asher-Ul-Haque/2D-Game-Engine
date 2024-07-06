#pragma once
#include "defines.h"


// - - - Memory Tages - - -


typedef enum memoryTag
{
    MEMORY_TAG_NONE,
    MEMORY_TAG_FILE,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_PILE_ALLOCATOR,
    MEMORY_TAG_LIST,
    MEMORY_TAG_DICTIONARY,
    MEMORY_TAG_CIRCULAR_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_TASK,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_MAX
} memoryTag;


// - - - | Memory Functions | - - -


// - - - Engine Memory Functions - - -


void memorySystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE);
void memorySystemShutdown();


// - - - Game Developer Memory Functions - - -

FORGE_API void* forgeAllocateMemory(unsigned long long SIZE, memoryTag TAG);

FORGE_API void forgeFreeMemory(void* MEMORY, unsigned long long SIZE, memoryTag TAG);

FORGE_API void forgeZeroMemory(void* MEMORY, unsigned long long SIZE);

FORGE_API void forgeCopyMemory(void* DESTINATION, const void* SOURCE, unsigned long long SIZE);

FORGE_API void forgeSetMemory(void* MEMORY, int VALUE, unsigned long long SIZE);

// - - - Debug Function
FORGE_API char* forgeGetMemoryStats();

FORGE_API unsigned long long forgeGetMemoryAllocCount();
