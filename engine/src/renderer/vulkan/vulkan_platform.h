#pragma once
#include "defines.h"


// - - - Platform Info - - - 

struct platformState;
struct vulkanContext;


// - - - | Platform Functions | - - -


void platformGetRequiredExtensions(const char*** EXTENSIONS);
/*
 Triple pointer?
 Well one pointer becuase a string is an array of characters.
 I want an array of strings, so I need a pointer to an array of pointers.
 I want to modify the array of strings, so I need a pointer to a pointer to an array of pointers.
*/

bool platformCreateSurface(struct vulkanContext* CONTEXT);
