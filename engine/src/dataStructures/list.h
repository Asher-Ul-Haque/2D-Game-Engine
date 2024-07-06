#pragma once
#include "defines.h"

/*
- - - | List structure | - - -
    unsigned long long CAPACITY : The maximum number of elements the list can hold
    unsigned long long LENGTH : The number of elements in the list
    unsigned long long STRIDE : The size of each element in bytes
    void* DATA : The data of the list, this is what is going to be returned
*/


enum
{
    LIST_CAPACITY,
    LIST_LENGTH,
    LIST_STRIDE,
    LIST_FIELD_LENGTH
};


// - - - List Controls - - - 

#define LIST_DEFAULT_CAPACITY 1
#define LIST_RESIZE_FACTOR 2


// - - - | List Functions | - - -


// - - - Private - - -

FORGE_API void* _listCreate(unsigned long long CAPACITY, unsigned long long STRIDE);
FORGE_API void _listDestroy(void* LIST);

FORGE_API unsigned long long _listGetField(void* LIST, unsigned long long FIELD);
FORGE_API void _listSetField(void* LIST, unsigned long long FIELD, unsigned long long VALUE);

FORGE_API void* _listResize(void* LIST);

FORGE_API void* _listAppend(void* LIST, const void* ELEMENT);
FORGE_API void _listPop(void* LIST, void* DESTINATION);

FORGE_API void* _listInsert(void* LIST, unsigned long long INDEX, const void* ELEMENT);
FORGE_API void* _listRemove(void* LIST, unsigned long long INDEX, void* DESTINATION);


// - - - Public - - -


#define listCreate(TYPE) \
    _listCreate(LIST_DEFAULT_CAPACITY, sizeof(TYPE))

#define listReserve(TYPE, CAPACITY) \
    _listCreate(CAPACITY, sizeof(TYPE))

#define listDestroy(LIST) \
    _listDestroy(LIST)

#define listAppend(LIST, VALUE)   \
    {                           \
        typeof(VALUE) temp = VALUE; \
        LIST = _listAppend(LIST, &temp); \
    }

#define listPop(LIST, DESTINATION) \
    _listPop(LIST, DESTINATION)

#define listInsert(LIST, INDEX, VALUE) \
    {                           \
        typeof(VALUE) temp = VALUE; \
        LIST = _listInsert(LIST, INDEX, &temp); \
    }

#define listRemove(LIST, INDEX, DESTINATION) \
    _listRemove(LIST, INDEX, DESTINATION)

#define listClear(LIST) \
    _listSetField(LIST, LIST_LENGTH, 0)

#define listCapacity(LIST) \
    _listGetField(LIST, LIST_CAPACITY)

#define listLength(LIST) \
    _listGetField(LIST, LIST_LENGTH)

#define listStride(LIST) \
    _listGetField(LIST, LIST_STRIDE)

#define listLengthSet(LIST, VALUE) \
    _listSetField(LIST, LIST_LENGTH, VALUE)
