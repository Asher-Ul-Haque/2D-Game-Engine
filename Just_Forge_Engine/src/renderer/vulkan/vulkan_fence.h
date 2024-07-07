#pragma once
#include "vulkan_types.h"

void vulkanFenceCreate(
    vulkanContext* CONTEXT,
    bool CREATE_SIGNAL,
    vulkanFence* OUTPUT_FENCE);

void vulkanFenceDestroy(vulkanContext* CONTEXT, vulkanFence* FENCE);

bool vulkanFenceWait(vulkanContext* CONTEXT, vulkanFence* FENCE, unsigned long long TIMEOUT);

void vulkanFenceReset(vulkanContext* CONTEXT, vulkanFence* FENCE);