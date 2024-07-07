#pragma once
#include "defines.h"


// - - - | Pile (Linear) Allocator | - - -

// - - - The Struct
typedef struct PileAllocator
{
    unsigned long long totalSize;
    unsigned long long allocatedSize;
    void* memory;
    bool ownsMemory;
} PileAllocator;


// - - - The functions - - -

// - - - Creation and Destruction
FORGE_API void createPileAllocator(unsigned long long TOTAL_SIZE, void* MEMORY, PileAllocator* ALLOCATOR);

FORGE_API void destroyPileAllocator(PileAllocator* ALLOCATOR);

// - - - Allocation and Freeing
FORGE_API void* pileAlloc(PileAllocator* ALLOCATOR, unsigned long long size);

FORGE_API void pileFree(PileAllocator* ALLOCATOR);
