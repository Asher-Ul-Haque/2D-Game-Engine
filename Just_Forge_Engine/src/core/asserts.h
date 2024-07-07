#pragma once
#include "defines.h"

// - - - Assertion enable toggle
#define FORGE_ASSERTS_ENABLED


// - - - | Assertions | - - -


#ifdef FORGE_ASSERTS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif


// - - - Assert Methods - - -

FORGE_API void reportAssertionFailure(const char* EXPRESSION, const char* MESSAGE, const char* FILE, int LINE);

#define FORGE_ASSERT(EXPRESSION)                \
    {                                           \
        if (EXPRESSION){}                       \
        else                                    \
        {                                       \
            reportAssertionFailure(#EXPRESSION, "", __FILE__, __LINE__);    \
            debugBreak();           \
        }                           \
    }                               

#define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE)    \
    {                                                \
        if (EXPRESSION){}                            \
        else                                         \
        {                                            \
            reportAssertionFailure(#EXPRESSION, MESSAGE, __FILE__, __LINE__);    \
            debugBreak();                            \
        }                                            \
    }

#ifdef _DEBUG
#define FORGE_ASSERT_DEBUG(EXPRESSION)               \
    {                                                \
        if (EXPRESSION){}                            \
        else                                         \
        {                                            \
            reportAssertionFailure(#EXPRESSION, "", __FILE__, __LINE__);    \
            debugBreak();                            \
        }                                            \
    }
#else 
#define KASSERT_DEBUG(EXPRESSION) //Does nothing at all
#endif

// - - - Assertion Disabled
#else
#define FORGE_ASSERT(EXPRESSION) //Does nothing at all
#define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE) //Does nothing at all
#define FORGE_ASSERT_DEBUG(EXPRESSION) //Does nothing at all
#endif
