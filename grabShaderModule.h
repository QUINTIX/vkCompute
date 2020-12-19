#include <vulkan/vulkan.h>
#include "tryCatch.h"

#ifndef GRAB_SHADER_MODULE_H
#define GRAB_SHADER_MODULE_H

VkShaderModule loadFromSPVFile(
        Ctx* context,
        VkDevice logicalDevice,
        VkShaderModuleCreateFlags flags,
        const char* fileName);

#endif
