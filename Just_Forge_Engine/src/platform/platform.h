#pragma once
#include "../defines.h"


// - - - | Platform Functions | - - -


// - - - State Functions - - -

bool platformSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APPLICATION, int X, int Y, int WIDTH, int HEIGHT);

void platformSystemShutdown(void* STATE);

bool platformGiveMessages();


// - - - Memory Functions - - -

void* platformAllocateMemory(unsigned long long SIZE, bool ALIGNED);

void platformFreeMemory(void* MEMORY, bool ALIGNED);

void* platformZeroMemory(void* MEMORY, unsigned long long SIZE);

void* platformCopyMemory(void* DESTINATION, const void* SOURCE, unsigned long long SIZE);

void* platformSetMemory(void* DESTINATION, int VALUE, unsigned long long SIZE);


// - - - Writing Functions - - - 

void platformWriteConsole(const char* MESSAGE, unsigned char COLOR);

void platformWriteConsoleError(const char* MESSAGE, unsigned char COLOR);


// - - - Time and Sleep Functions - - -

void platformSleep(unsigned long long MILLISECONDS);

double platformGetTime();
