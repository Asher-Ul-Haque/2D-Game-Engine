#include "pile_alloc.h"
#include "core/memory.h"
#include "core/logger.h"


// - - - | Pile (Linear) Allocator | - - -


// - - - The functions - - -

// - - - Creation and Destruction
void createPileAllocator(unsigned long long TOTAL_SIZE, void* MEMORY, PileAllocator* ALLOCATOR)
{
    if (ALLOCATOR)
    {
        ALLOCATOR->totalSize = TOTAL_SIZE;
        ALLOCATOR->allocatedSize = 0;
        ALLOCATOR->ownsMemory = (MEMORY == 0);

        if (MEMORY)
        {
            ALLOCATOR->memory = MEMORY;
        }
        else 
        {
            ALLOCATOR->memory = forgeAllocateMemory(TOTAL_SIZE, MEMORY_TAG_PILE_ALLOCATOR);
        }
    }
}

FORGE_API void destroyPileAllocator(PileAllocator* ALLOCATOR)
{
    if (ALLOCATOR)
    {
        ALLOCATOR->totalSize = 0;
        ALLOCATOR->allocatedSize = 0;
        ALLOCATOR->ownsMemory = (ALLOCATOR->memory == 0);

        if (ALLOCATOR->memory && ALLOCATOR->ownsMemory)
        {
            forgeFreeMemory(ALLOCATOR->memory, ALLOCATOR->totalSize, MEMORY_TAG_PILE_ALLOCATOR);
        }
        ALLOCATOR->memory = 0;
        ALLOCATOR->totalSize = 0;
        ALLOCATOR->ownsMemory = false;
    }
}

// - - - Allocation and Freeing
FORGE_API void* pileAlloc(PileAllocator* ALLOCATOR, unsigned long long SIZE)
{
    if (ALLOCATOR && ALLOCATOR->memory)
    {
        if (ALLOCATOR->allocatedSize + SIZE > ALLOCATOR->totalSize)
        {
            unsigned long long remaining = ALLOCATOR->totalSize - ALLOCATOR->allocatedSize;
            FORGE_LOG_ERROR("Ran out of memory while trying to allocate in pileAllcator.\nRemaining: %lluB \t\t Requested: %lluB", remaining, SIZE);
            return 0;
        }

        void* block = ALLOCATOR->memory + ALLOCATOR->allocatedSize;
        ALLOCATOR->allocatedSize += SIZE;

        return block;
    }
    return 0;
}

FORGE_API void pileFree(PileAllocator* ALLOCATOR)
{
    if (ALLOCATOR && ALLOCATOR->memory)
    {
        ALLOCATOR->allocatedSize = 0;
        forgeZeroMemory(ALLOCATOR->memory, ALLOCATOR->totalSize);
    }
}
