#include "dataStructures/list.h"
#include "core/memory.h"
#include "core/logger.h"
#include "list.h"


// - - - | List Functions | - - -


// - - - Creation and Destruction - - -

void* _listCreate(unsigned long long CAPACITY, unsigned long long STRIDE)
{
    unsigned long long headerSize = LIST_FIELD_LENGTH * sizeof(unsigned long long);
    unsigned long long dataSize = CAPACITY * STRIDE;
    unsigned long long* list = forgeAllocateMemory(headerSize + dataSize, MEMORY_TAG_LIST);
    forgeSetMemory(list, 0, headerSize + dataSize);
    list[LIST_CAPACITY] = CAPACITY;
    list[LIST_LENGTH] = 0;
    list[LIST_STRIDE] = STRIDE;
    return (void*) (list + LIST_FIELD_LENGTH);
}

void _listDestroy(void* LIST)
{
    unsigned long long* header = (unsigned long long*) LIST - LIST_FIELD_LENGTH;
    unsigned long long headerSize = LIST_FIELD_LENGTH * sizeof(unsigned long long);
    unsigned long long totalSize = headerSize + header[LIST_CAPACITY] * header[LIST_STRIDE];
}


// - - - Accessors - - -

unsigned long long _listGetField(void* LIST, unsigned long long FIELD)
{
    unsigned long long* header = (unsigned long long*) LIST - LIST_FIELD_LENGTH;
    return header[FIELD];
}

void _listSetField(void* LIST, unsigned long long FIELD, unsigned long long VALUE)
{
    unsigned long long* header = (unsigned long long*) LIST - LIST_FIELD_LENGTH;
    header[FIELD] = VALUE;
}

void* _listResize(void* LIST)
{
    unsigned long long length = listLength(LIST);
    unsigned long long stride = listStride(LIST);
    void* list = _listCreate(listCapacity(LIST) * LIST_RESIZE_FACTOR, stride);
    forgeCopyMemory(list, LIST, length * stride);

    _listSetField(list, LIST_LENGTH, length);
    _listDestroy(LIST);

    return list;
}


// - - - Element Manipulation - - -

void* _listAppend(void* LIST, const void* ELEMENT)
{
    unsigned long long length = listLength(LIST);
    unsigned long long capacity = listCapacity(LIST);
    unsigned long long stride = listStride(LIST);
    if (length >= capacity)
    {
        LIST = _listResize(LIST);
    }
    unsigned long long address = (unsigned long long) LIST;
    address += (length * stride);
    forgeCopyMemory((void*) address, ELEMENT, stride);
    _listSetField(LIST, LIST_LENGTH, length + 1);
    return LIST;
}

void _listPop(void* LIST, void* DESTINATION)
{
    unsigned long long length = listLength(LIST);
    unsigned long long stride = listStride(LIST);
    unsigned long long address = (unsigned long long) LIST;
    address += ((length - 1) * stride);
    forgeCopyMemory(DESTINATION, (void*) address, stride);
    _listSetField(LIST, LIST_LENGTH, length - 1);
}

void* _listInsert(void* LIST, unsigned long long INDEX, const void* ELEMENT)
{
    unsigned long long length = listLength(LIST);
    unsigned long long stride = listStride(LIST);
    if (INDEX >= length)
    {
        FORGE_LOG_ERROR("Index out of bounds of this list! length: %llu, index: %llu", length, INDEX);
        return LIST;
    }
    if (length >= listCapacity(LIST))
    {
        LIST = _listResize(LIST);
    }

    unsigned long long address = (unsigned long long) LIST;

    if (INDEX != length - 1)
    {
        forgeCopyMemory((void*) (address + ((INDEX + 1) * stride)), 
                        (void*) (address + (INDEX * stride)), 
                        stride * (length - INDEX));
    }
    forgeCopyMemory((void*) (address + (INDEX * stride)), ELEMENT, stride);
    _listSetField(LIST, LIST_LENGTH, length + 1);
    return LIST;
}

void* _listRemove(void* LIST, unsigned long long INDEX, void* DESTINATION)
{
    unsigned long long length = _listGetField(LIST, LIST_LENGTH);
    unsigned long long stride = _listGetField(LIST, LIST_STRIDE);
    if (INDEX >= length)
    {
        FORGE_LOG_ERROR("Index out of bounds of this list! length: %llu, index: %llu", length, INDEX);
        return LIST;
    }
    unsigned long long address = (unsigned long long) LIST;
    forgeCopyMemory(DESTINATION, 
                    (void*) (address + (INDEX * stride)), 
                    stride);

    if (INDEX != length - 1)
    {
        forgeCopyMemory(
            (void*) (address + (INDEX * stride)), 
            (void*) (address + ((INDEX + 1) * stride)), 
            stride * (length - INDEX));
        address += (length - 1) * stride;
    }

    _listSetField(LIST, LIST_LENGTH, length - 1);
    return LIST;
}

