#include "core/input.h"
#include "core/memory.h"
#include "core/logger.h"
#include "core/event.h"
#include "event.h"
#include "input.h"


// - - - | Input Tracking | - - -


// - - - Keyboard
typedef struct keyBoardState
{
    bool keys[256];
} keyBoardState;

// - - - Mouse
typedef struct mouseState
{
    unsigned char buttons[BUTTON_MAX_BUTTONS];
    short x, y;
} mouseState;

// - - - Input State 
typedef struct inputState
{
    keyBoardState keyBoardCurrent;
    keyBoardState keyBoardPrevious;
    mouseState mouseCurrent;
    mouseState mousePrevious;
} inputState;

// - - - Input State
static inputState* statePtr;


// - - - | Input Functions | - - -


// - - - System Functions - - -

void inputSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE)
{
    *MEMORY_REQUIREMENT = sizeof(inputState);
    if (STATE == 0)
    {
        return;
    }
    forgeZeroMemory(STATE, sizeof(inputState));
    statePtr = STATE;
    FORGE_LOG_INFO("Input Subsystem initialized");
}

void inputSystemShutdown(void* STATE)
{
    //TODO: Shutdown routines
    statePtr = 0;
    FORGE_LOG_INFO("Input system shutdown");
}

void inputUpdate(double DELTA_TIME)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return;
    }
    
    //Copy current state to previous state 
    forgeCopyMemory(&statePtr->keyBoardPrevious, &statePtr->keyBoardCurrent, sizeof(keyBoardState));
    forgeCopyMemory(&statePtr->mousePrevious, &statePtr->mouseCurrent, sizeof(mouseState));
}


// - - - Keyboard Functions - - -

// - - - Key Functions
bool inputIsKeyDown(keys KEY)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return false;
    }
    return statePtr->keyBoardCurrent.keys[KEY] == true;
}

bool inputIsKeyUp(keys KEY)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return true;
    }
    return statePtr->keyBoardCurrent.keys[KEY] == false;
}

bool inputWasKeyDown(keys KEY)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return false;
    }
    return statePtr->keyBoardPrevious.keys[KEY] == true;
}

bool inputWasKeyUp(keys KEY)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return false;
    }
    return statePtr->keyBoardPrevious.keys[KEY] == false;
}

// - - - Input Processing Functions
void inputProcessKey(keys KEY, bool IS_DOWN)
{
    if (statePtr->keyBoardCurrent.keys[KEY] != IS_DOWN)
    {
        statePtr->keyBoardCurrent.keys[KEY] = IS_DOWN;

        eventContext context;
        context.data.u16[0] = KEY;
        eventTrigger(IS_DOWN ? EVENT_CODE_KEY_PRESS : EVENT_CODE_KEY_RELEASE, 0, context);
    }
}


// - - - Mouse Functions - - -

// - - - Button Functions
bool inputIsButtonDown(buttons BUTTON)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return false;
    }
    return statePtr->mouseCurrent.buttons[BUTTON] == true;
}

bool inputIsButtonUp(buttons BUTTON)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return true;
    }
    return statePtr->mouseCurrent.buttons[BUTTON] == false;
}

bool inputWasButtonDown(buttons BUTTON)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return false;
    }
    return statePtr->mousePrevious.buttons[BUTTON] == true;
}

bool inputWasButtonUp(buttons BUTTON)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Input system not initialized");
        return true;
    }
    return statePtr->mousePrevious.buttons[BUTTON] == false;
}

void inputGetMousePosition(int *X, int *Y)
{
    if (!statePtr)
    {
        *X = 0;
        *Y = 0;
        FORGE_LOG_WARNING("Input system not initialized");
        return;
    }
    *X = statePtr->mouseCurrent.x;
    *Y = statePtr->mouseCurrent.y;
}

void inputGetPreviousMousePosition(int *X, int *Y)
{
    if (!statePtr)
    {
        *X = 0;
        *Y = 0;
        FORGE_LOG_WARNING("Input system not initialized");
        return;
    }
    *X = statePtr->mousePrevious.x;
    *Y = statePtr->mousePrevious.y;
}

// - - - Input Processing Functions
void inputProcessButton(buttons BUTTON, bool IS_DOWN)
{
    if (statePtr->mouseCurrent.buttons[BUTTON] != IS_DOWN)
    {
        statePtr->mouseCurrent.buttons[BUTTON] = IS_DOWN;

        eventContext context;
        context.data.u16[0] = BUTTON;
        eventTrigger(IS_DOWN ? EVENT_CODE_BUTTON_PRESS : EVENT_CODE_BUTTON_RELEASE, 0, context);
    }
}

void inputProcessMouseMovement(short X, short Y)
{
    if (statePtr->mouseCurrent.x != X || statePtr->mouseCurrent.y != Y)
    {
        FORGE_LOG_TRACE("Mouse Position: %i, %i", X, Y);

        statePtr->mouseCurrent.x = X;
        statePtr->mouseCurrent.y = Y;

        eventContext context;
        context.data.u16[0] = X;
        context.data.u16[1] = Y;
        eventTrigger(EVENT_CODE_MOUSE_MOVE, 0 , context);
    }
}

void inputProcessMouseWheel(char WHEEL_DELTA)
{
    // NOTE: no internal state to update
    eventContext context;
    context.data.u8[0] = WHEEL_DELTA;
    eventTrigger(EVENT_CODE_MOUSE_WHEEL, 0, context);
}
