#pragma once
#include "vulkan_types.h"


// - - - Basic Vulkan Utility Functions - - - 

/**
 * Returns the string representation of RESULT.
 * @param RESULT The RESULT to get the string for.
 * @param EXTEND Indicates whether to also return an extended RESULT.
 * @returns The error code and/or extended error message in string form. Defaults to success for unknown RESULT types.
 */
const char* vulkanResultToString(VkResult RESULT, bool EXTEND);

/**
 * Inticates if the passed RESULT is a success or an error as defined by the Vulkan spec.
 * @returns True if success; otherwise false. Defaults to true for unknown RESULT types.
 */
bool vulkanResultIsSuccess(VkResult RESULT);