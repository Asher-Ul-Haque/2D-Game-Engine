#include "memory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "string.h"
#include "stdio.h"


// - - - Memory Stats - - -

struct memoryStats
{
    unsigned long long totalAllocated;
    unsigned long long taggedAllocated[MEMORY_TAG_MAX];
};

typedef struct memorySystemState
{
    struct memoryStats stats;
    unsigned long long allocCount;
} memorySystemState;

static memorySystemState* statePtr;


// - - - String representation of tags
static const char* memoryTagAsStrings[MEMORY_TAG_MAX] = {
    "NONE           ",
    "FILE           ",
    "ARRAY          ",
    "PILE ALLOCATOR ",
    "LIST           ",
    "DICTIONARY     ",
    "CIRCULAR QUEUE ",
    "BST            ",
    "APPLICATION    ",
    "TASK           ",
    "TEXTURE        ",
    "MATERIAL       ",
    "RENDERER       ",
    "GAME           ",
    "TRANSFORM      ",
    "ENTITY         ",
    "ENTITY NODE    ",
    "SCENE          "};


// - - - | Memory Functions | - - -


// - - - Engine Memory Functions - - -

void memorySystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE)
{
    *MEMORY_REQUIREMENT = sizeof(memorySystemState);
    if (STATE == 0)
    {
        return;
    }
    statePtr = STATE;
    statePtr->allocCount = 0;
    platformZeroMemory(&statePtr->stats, sizeof(statePtr->stats));
    FORGE_LOG_INFO("Memory Initialized");
}

void memorySystemShutdown()
{
    FORGE_LOG_INFO("Memory Shutdown");
    //TODO: cleanup of memory applications
    statePtr = 0;
}


// - - - Game Developer Memory Functions - - -

void* forgeAllocateMemory(unsigned long long SIZE, memoryTag TAG)
{
    if (TAG == MEMORY_TAG_NONE)
    {
        FORGE_LOG_WARNING("forgeAllocateMemory called using MEMORY_TAG_NONE. Recommended to use only tagged allocations");
    }
    
    if (statePtr)
    {
        statePtr->stats.totalAllocated += SIZE;
        statePtr->stats.taggedAllocated[TAG] += SIZE;
        ++statePtr->allocCount;
    }

    // TODO: memory allignment
    void* memoryBlock = platformAllocateMemory(SIZE, false);
    platformZeroMemory(memoryBlock, SIZE);

    return memoryBlock;
}

void forgeFreeMemory(void* MEMORY, unsigned long long SIZE, memoryTag TAG)
{
    if (TAG == MEMORY_TAG_NONE)
    {
        FORGE_LOG_WARNING("forgeAllocateMemory called using MEMORY_TAG_NONE. Recommended to use only tagged allocations");
    }
    
    if (statePtr)
    {
        statePtr->stats.totalAllocated -= SIZE;
        statePtr->stats.taggedAllocated[TAG] -= SIZE;
    }

    // TODO: memory allignment
    platformFreeMemory(MEMORY, false);
}

void forgeZeroMemory(void* MEMORY, unsigned long long SIZE)
{
    platformZeroMemory(MEMORY, SIZE);
}

void forgeCopyMemory(void* DESTINATION, const void* SOURCE, unsigned long long SIZE)
{
    platformCopyMemory(DESTINATION, SOURCE, SIZE);
}

void forgeSetMemory(void* MEMORY, int VALUE, unsigned long long SIZE)
{
    platformSetMemory(MEMORY, VALUE, SIZE);
}

// - - - Debug Function
char* forgeGetMemoryStats()
{
    const unsigned long long kb = 1024; //Cant allocated less than a byte
    const unsigned long long mb = kb * 1024;
    const unsigned long long gb = mb * 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    unsigned long long offset = strlen(buffer);
    float amount = 1.0f;
    char unit[3] = "XB";
    for (int i = 0; i < MEMORY_TAG_MAX; ++i)
    {
        if (statePtr->stats.taggedAllocated[i] >= gb)
        {
            unit[0] = 'G';
            unit[1] = 'B';
            amount = statePtr->stats.taggedAllocated[i] / (float) gb;
        }
        else if (statePtr->stats.taggedAllocated[i] >= mb)
        {
            unit[0] = 'M';
            unit[1] = 'B';
            amount = statePtr->stats.taggedAllocated[i] / (float) mb;
        }
        else if (statePtr->stats.taggedAllocated[i] >= kb)
        {
            unit[0] = 'K';
            unit[1] = 'B';
            amount = statePtr->stats.taggedAllocated[i] / (float) kb;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = 0; //Terminate string
            amount = statePtr->stats.taggedAllocated[i];
        }

        int length = snprintf(buffer + offset, 8000, "  %s: %.2f %s\n", memoryTagAsStrings[i], amount, unit);
        offset += length;

        //Add a total memory allocation
    }
    unsigned long long total = statePtr->stats.totalAllocated;
    if (total >= gb)
    {
        unit[0] = 'G';
        unit[1] = 'B';
        amount = total / (float) gb;
    }
    else if (total >= mb)
    {
        unit[0] = 'M';
        unit[1] = 'B';
        amount = total / (float) mb;
    }
    else if (total >= kb)
    {
        unit[0] = 'K';
        unit[1] = 'B';
        amount = total / (float) kb;
    }
    else
    {
        unit[0] = 'B';
        unit[1] = 0; //Terminate string
        amount = total;
    }
    snprintf(buffer + offset, 8000, "  Total: %.2f %s\n", amount, unit);

    char* outputString = stringDuplicate(buffer);
    return outputString;
}

unsigned long long forgeGetMemoryAllocCount()
{
    if (statePtr)
    {
        return statePtr->allocCount;
    }
    return 0;
}
