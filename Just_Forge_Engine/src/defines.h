#pragma once


// - - - | New Data Types and Assertions | - - -


// - - - Boolean Data Types
typedef int boolean;
typedef _Bool bool;

#define true 1
#define false 0

// - - - Static Assertions
#if defined (__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

//Ensure all types are of the correct size
// - - - Boolean
STATIC_ASSERT(sizeof(boolean) == 4, "boolean32 is not 4 bytes");
STATIC_ASSERT(sizeof(bool) == 1, "boolean8 is not 1 byte");

// - - - Character
STATIC_ASSERT(sizeof(char) == 1, "char is not 1 byte");
STATIC_ASSERT(sizeof(unsigned char) == 1, "unsigned char is not 1 byte");

// - - - Smol integer
STATIC_ASSERT(sizeof(short) == 2, "short is not 2 bytes");
STATIC_ASSERT(sizeof(unsigned short) == 2, "unsigned short is not 2 bytes");

// - - - Normal integer
STATIC_ASSERT(sizeof(int) == 4, "int is not 4 bytes");
STATIC_ASSERT(sizeof(unsigned int) == 4, "unsigned int is not 4 bytes");

// - - - Beeg Integer
STATIC_ASSERT(sizeof(long long) == 8, "long long is not 8 bytes");
STATIC_ASSERT(sizeof(unsigned long long) == 8, "unsigned long long is not 8 bytes");

// - - - Floating point numbers
STATIC_ASSERT(sizeof(float) == 4, "float is not 4 bytes");
STATIC_ASSERT(sizeof(double) == 8, "double is not 8 bytes");


// - - - | Platform Detection | - - -


// - - - Windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define FORGE_PLATFORM_WINDOWS 1
#define stringDuplicate _strdup

// - - - Linux
#elif defined(__linux__) || defined(__gnu_linux__)
#define FORGE_PLATFORM_LINUX 1
#define stringDuplicate strdup

#if defined(__ANDROID__)
#define FORGE_PLATFORM_ANDROID 1 
#endif

#elif defined(__unix__)
#define FORGE_PLATFORM_UNIX 1 
#elif degined(_POSIX_VERSION)
#define FORGE_PLATFORM_POSIX 1 

// - - - MacOS
#elif defined(__APPLE__) || defined(__MACH__)
#define FORGE_PLATFORM_MACOS 1 
#include <TargetConditionals.h>

#if TARGET_IPHONE_SIMULATOR 
//iOS Simulator
#define FORGE_PLATFORM_IOS 1
#define FORGE_PLATFORM_IOS_SIMULATOR 1 

#elif TARGET_OS_IPHONE
#define FORGE_PLATFORM_IOS 1 

#elif TARGET_OS_MAC
#define FORGE_PLATFORM_MACOS 1 
#else 
#error "Unknown Apple platform" 
#endif 
#else 
#error "Unknown platform!"
#endif


// - - - | Forge API | - - -

// - - - Release
#ifdef _DEBUG
#define FORGE_RELEASE 0
#else 
//#define FORGE_RELEASE 1
#endif


// - - - Export
#ifdef FORGE_EXPORT
#ifdef _MSC_VER
#define FORGE_API __declspec(dllexport)
#else 
#define FORGE_API __attribute__((visibility("default")))
#endif

// - - - Imports
#else 
#ifdef _MSC_VER
#define FORGE_API __declspec(dllimport)
#else 
#define FORGE_API
#endif
#endif

#define FORGE_CLAMP(VALUE, MIN, MAX) ((VALUE) <= (MIN) ? (MIN) : (VALUE) >= (MAX) ? (MAX) : (VALUE))

// - - - Inlining
#ifdef _MSC_VER
#define FORGE_INLINE __forceinline
#define FORGE_NO_INLINE __declspec(noinline)
#else 
#define FORGE_INLINE static inline
#define FORGE_NO_INLINE
#endif
