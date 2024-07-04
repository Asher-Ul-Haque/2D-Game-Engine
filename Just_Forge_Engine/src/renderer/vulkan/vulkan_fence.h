#pragma once
#include "vulkan_types.h"

void createFence(vulkanContext* CONTEXT, bool createSignal, vulkanFence* FENCE);

void destroyFence(vulkanContext* CONTEXT, vulkanFence* FENCE);

bool waitForFence(vulkanContext* CONTEXT, vulkanFence* FENCE, unsigned long long TIMEOUT);

void resetFence(vulkanContext* CONTEXT, vulkanFence* FENCE);
