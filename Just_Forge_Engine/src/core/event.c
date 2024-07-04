#include "core/event.h"
#include "core/memory.h"
#include "core/logger.h"
#include "dataStructures/list.h"
#include "logger.h"

typedef struct registeredEvent
{
    void* listener;
    eventCallback callback;
} registeredEvent;

typedef struct eventCodeEntry
{
    registeredEvent* events;
} eventCodeEntry;

// This should be enough number of codes
#define MAX_MESSAGE_CODES 16384

//State structure
typedef struct eventSystemState
{
    eventCodeEntry registry[MAX_MESSAGE_CODES];
} eventSystemState;

static eventSystemState* statePtr;


// - - - | Event System functions | - - -


// - - - Game engine functions - - -

void eventSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void * STATE)
{
    *MEMORY_REQUIREMENT = sizeof(eventSystemState);
    if (STATE == 0)
    {
        return;
    }
    forgeZeroMemory(STATE, sizeof(STATE));
    statePtr = STATE;
    FORGE_LOG_INFO("Event system initialized");
}

void eventSystemShutdown(void* STATE)
{
    if (statePtr)
    {
        // Free the events arrays.And objects pointed to should be self destroyed
        for (unsigned short i = 0; i < MAX_MESSAGE_CODES; ++i)
        {
            if (statePtr->registry[i].events != 0)
            {
                //Print out the registry[i].events list
                listDestroy(statePtr->registry[i].events);
                statePtr->registry[i].events = 0;
            }
        }
    }
    statePtr = 0;
    FORGE_LOG_INFO("Event system shut down");
}


// - - - Game developer functions - - -

bool eventRegister(unsigned short CODE,  void* LISTENER, eventCallback CALLBACK)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Event system not initialized");
        return false;
    }

    if (statePtr->registry[CODE].events == 0)
    {
        statePtr->registry[CODE].events = listCreate(registeredEvent);
    }

    unsigned long long registeredCount = listLength(statePtr->registry[CODE].events);
    for (unsigned long long i = 0; i < registeredCount; ++i)
    {
        if (statePtr->registry[CODE].events[i].listener == LISTENER)
        {
            FORGE_LOG_WARNING("Listener already registered for event code %i", CODE);
            return false;
        }
    }

    //Safe to register
    registeredEvent event;
    event.listener = LISTENER;
    event.callback = CALLBACK;
    listAppend(statePtr->registry[CODE].events, event);

    return true;
}

bool eventUnregister(unsigned short CODE, void* LISTENER, eventCallback CALLBACK)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Event system not initialized");
        return false;
    }

    if (statePtr->registry[CODE].events == 0)
    {
        FORGE_LOG_WARNING("No listeners registered for event code %i", CODE);
        return false;
    }

    unsigned long long registeredCount = listLength(statePtr->registry[CODE].events);

    for (unsigned long long i = 0; i < registeredCount; ++i)
    {
        registeredEvent event = statePtr->registry[CODE].events[i];
        if (event.listener == LISTENER && event.callback == CALLBACK)
        {
            registeredEvent poppedEvent;
            listRemove(statePtr->registry[CODE].events, i, &poppedEvent);
            return true;
        }
    }

    //Not found
    FORGE_LOG_WARNING("No event was registered for code %i with listener %p", CODE, LISTENER);
    return false;
}

bool eventTrigger(unsigned short CODE, void* SENDER, eventContext CONTEXT)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Event system not initialized");
        return false;
    }

    if (statePtr->registry[CODE].events == 0)
    {
        FORGE_LOG_WARNING("No listeners to trigger for event code %i", CODE);
        return false;
    }

    unsigned long long registeredCount = listLength(statePtr->registry[CODE].events);
    for (unsigned long long i = 0; i < registeredCount; ++i)
    {
        registeredEvent event = statePtr->registry[CODE].events[i];
        if (event.callback(CODE, SENDER, event.listener, CONTEXT))
        {
            //Message has been handled, no need to send for other listeners
            return true;
        }
    }
    
    //Not found
    return false;
}
