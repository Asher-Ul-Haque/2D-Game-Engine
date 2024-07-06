#pragma once
#include "defines.h"

struct platform_state;
struct vulkanContext;

bool platformCreateVulkanSurface(struct vulkanContext* CONTEXT);

/**
 * Appends the names of required extensions for this platform to
 * the NAMES_LIST, which should be created and passed in.
 */
void platformGetRequiredExtensionNames(const char*** NAMES_LIST);