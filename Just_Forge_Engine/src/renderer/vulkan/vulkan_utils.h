#pragma once
#include "vulkan_types.h"

const char* vulkanResultToString(VkResult RESULT, bool GET_MORE);

bool resultSuccess(VkResult RESULT);
