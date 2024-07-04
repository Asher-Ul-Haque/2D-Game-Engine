#include "application.h"
#include "game_types.h"

#include "logger.h"

#include "platform/platform.h"

#include "core/memory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/pile_alloc.h"

#include "renderer/renderer_frontend.h"


// - - - | Application State | - - -


typedef struct applicationState
{
    game* gameInstance;
    bool isRunning;
    bool isSuspended;
    short width;
    short height;
    double lastTime;
    clock clock;
    PileAllocator systemsAllocator;

    unsigned long long loggerSystemMemoryRequirement;
    void* loggerSystemState;

    unsigned long long memorySystemMemoryRequirement;
    void* memorySystemState;
    
    unsigned long long eventSystemMemoryRequirement;
    void* eventSystemState;

    unsigned long long inputSystemMemoryRequirement;
    void* inputSystemState;

    unsigned long long platformSystemMemoryRequirement;
    void* platformSystemState;

    unsigned long long rendererSystemMemoryRequirement;
    void* rendererSystemState;
} applicationState;

static applicationState* appState;


// - - - | Application Functions | - - -


// - - - Event Handlers - - -

bool applicationOnEvent(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT);

bool applicationOnKey(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT);

bool applicationOnResize(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT);


// - - - Create Application
bool createApplication(game* GAME)
{
    if (GAME->applicationState)
    {
        FORGE_LOG_ERROR("Application cannot be created more than once");
        return false;
    }
    
    GAME->applicationState = forgeAllocateMemory(sizeof(applicationState), MEMORY_TAG_APPLICATION);
    appState = GAME->applicationState;
    appState->gameInstance = GAME;
    appState->isRunning = false;
    appState->isSuspended = false;

    //Initialize pile allocator
    unsigned long long systemAllocTotalSize =  1 * 1024 * 1024; // 1mb
    createPileAllocator(systemAllocTotalSize, 0, &appState->systemsAllocator);

    //Initialise event system
    eventSystemInitialize(&appState->eventSystemMemoryRequirement, 0);
    appState->eventSystemState = pileAlloc(&appState->systemsAllocator, appState->eventSystemMemoryRequirement);
    eventSystemInitialize(&appState->eventSystemMemoryRequirement, appState->eventSystemState);

    //Initialize memory system
    memorySystemInitialize(&appState->memorySystemMemoryRequirement, 0);
    appState->memorySystemState = pileAlloc(&appState->systemsAllocator, appState->memorySystemMemoryRequirement);
    memorySystemInitialize(&appState->memorySystemMemoryRequirement, appState->memorySystemState);

    //Initialise logging system
    initializeLogger(&appState->loggerSystemMemoryRequirement, 0);
    appState->loggerSystemState = pileAlloc(&appState->systemsAllocator, appState->loggerSystemMemoryRequirement);
    if (!initializeLogger(&appState->loggerSystemMemoryRequirement, appState->loggerSystemState))
    {
        FORGE_LOG_ERROR("Failed to initialize logging system, shutting down!");
        return false;
    }

    //Intialise input system
    inputSystemInitialize(&appState->inputSystemMemoryRequirement, 0);
    appState->inputSystemState = pileAlloc(&appState->systemsAllocator, appState->inputSystemMemoryRequirement);
    inputSystemInitialize(&appState->inputSystemMemoryRequirement, appState->inputSystemState);

    //Register event listeners
    eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESS, 0, applicationOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASE, 0, applicationOnKey);
    eventRegister(EVENT_CODE_RESIZE, 0, applicationOnResize);

    //Intitialise the platform
    platformSystemInitialize(&appState->platformSystemMemoryRequirement, 0, 0, 0, 0, 0, 0);
    appState->platformSystemState = pileAlloc(&appState->systemsAllocator, appState->platformSystemMemoryRequirement);
    if (!platformSystemInitialize(&appState->platformSystemMemoryRequirement, appState->platformSystemState, GAME->config.name, GAME->config.startPositionX, GAME->config.startPositionY, GAME->config.startWidth, GAME->config.startHeight))
    {
        FORGE_LOG_FATAL("Failed to initialize platform system");
        return false;
    }
    
    //Initialise the renderer
    rendererSystemInitialize(&appState->platformSystemMemoryRequirement, 0, GAME->config.name);
    appState->rendererSystemState = pileAlloc(&appState->systemsAllocator, appState->rendererSystemMemoryRequirement);
    rendererSystemInitialize(&appState->rendererSystemMemoryRequirement, appState->rendererSystemState, GAME->config.name);
    
    //Initialise the game
    if (!appState->gameInstance->init(appState->gameInstance))
    {
        FORGE_LOG_FATAL("Failed to initialise game");
        return false;
    }

    appState->gameInstance->onResize(appState->gameInstance, appState->width, appState->height);

    return true;
}

// - - - Run Application
bool runApplication()
{
    appState->isRunning = true;
    clockStart(&appState->clock);
    clockUpdate(&appState->clock);
    appState->lastTime = appState->clock.elapsedTime;
    double runningTime = 0.0;
    unsigned char frameCount = 0;
    double targetFrameTime = 1.0f / 60.0; // 60 fps

    while (appState->isRunning) 
    {
        if (!platformGiveMessages())
        {
            appState->isRunning = false;
        }

        // TODO: take care of delta time.
        if (!appState->isSuspended)
        {
            clockUpdate(&appState->clock);
            double currentTime = appState->clock.elapsedTime;
            double deltaTime = currentTime - appState->lastTime;
            double frameStartTime = platformGetTime();

            if (!appState->gameInstance->update(appState->gameInstance, deltaTime))
            {
                FORGE_LOG_FATAL("Failed to update game");
                appState->isRunning = false;
                break;
            }

            if (!appState->gameInstance->render(appState->gameInstance, deltaTime))
            {
                FORGE_LOG_FATAL("Failed to render game");
                appState->isRunning = false;
                break;
            }

            // TODO: refctor packet creation
            rendererPacket packet;
            packet.deltaTime = deltaTime;
            rendererDrawFrame(&packet);  

            double frameEndTime = platformGetTime();
            double frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            double remainingSeconds = targetFrameTime - frameElapsedTime;

            if (remainingSeconds > 0)
            {
                unsigned long long remainingMiliseconds = (remainingSeconds * 1000);
                bool limitFrames = false; //give time back to os
                if (remainingMiliseconds > 0 && limitFrames)
                {
                    platformSleep(remainingMiliseconds - 1);
                }
                ++frameCount;
            }

            //NOTE: input state should be handled after input recorded or before this line
            inputUpdate(deltaTime);
            
            appState->lastTime = currentTime;
        }
    }

    appState->isRunning = false;

    //Unregister event listeners
    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESS, 0, applicationOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASE, 0, applicationOnKey);

    inputSystemShutdown(appState->inputSystemState);
    rendererSystemShutdown(appState->rendererSystemState);
    platformSystemShutdown(appState->platformSystemState);
    memorySystemShutdown(appState->platformSystemState);
    eventSystemShutdown(appState->eventSystemState);
    return true;
}

// - - - Get Frame Buffer Size
void applicationGetFrameBufferSize(unsigned int* WIDTH, unsigned int* HEIGHT)
{
    *WIDTH = appState->width;
    *HEIGHT = appState->height;
}


// - - - Event Handlers - - -

bool applicationOnEvent(unsigned short CODE, void* SENDER, void* LISTENER, eventContext context)
{
    switch (CODE)
    {
        case EVENT_CODE_APPLICATION_QUIT:
            FORGE_LOG_INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down");
            appState->isRunning = false;
            return true;

        default:
            return false;
    }

    return false;
}

bool applicationOnKey(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT)
{
    unsigned short keyCode;
    eventContext context = {};
    switch (CODE)
    {
        case EVENT_CODE_KEY_PRESS:
            keyCode = CONTEXT.data.u16[0];
            switch (keyCode)
            {
                case KEY_ESCAPE:
                    eventTrigger(EVENT_CODE_APPLICATION_QUIT, 0, context);
                    return true;

                default:
                    FORGE_LOG_TRACE("Key %i pressed", keyCode);
                    return false;
            }
            break;

        case EVENT_CODE_KEY_RELEASE:
            keyCode = CONTEXT.data.u16[0];
            switch (keyCode)
            {
                default:
                    FORGE_LOG_TRACE("Key %i released", keyCode);
                    return false;
            }
            break;
    }
    return false;
}


bool applicationOnResize(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT)
{
    switch (CODE)
    {
        case EVENT_CODE_RESIZE:
        {
            unsigned short width = CONTEXT.data.u16[0];
            unsigned short height = CONTEXT.data.u16[1];

            if (width != appState->width || height != appState->height)
            {
                appState->width = width;
                appState->height = height;

                FORGE_LOG_DEBUG("Window resized to %i x %i", width, height);

                //Minimization
                if (width <= 10 || height <= 10)
                {
                    FORGE_LOG_INFO("Window minimized...Suspending game instance");
                    appState->isSuspended = true;
                    return true;
                }
                else
                {
                    if (appState->isSuspended)
                    {
                        FORGE_LOG_INFO("Window restored...Resuming game instance");
                        appState->isSuspended = false;
                    }
                    appState->gameInstance->onResize(appState->gameInstance, width, height);
                    rendererResized(width, height);
                }
            }
        }
    }
    return false;
}
