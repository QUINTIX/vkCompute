#include "vulkan/vulkan.h"
#include "tryCatch.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef FIND_FOR_INIT_H
#define FIND_FOR_INIT_H

static Ctx ctx; /* ctx is explicitly _not_ shared between c files,
if static does not make that abundantly clear */

#define RAISE_ON_BAD_RESULT(result) \
    if (VK_SUCCESS != (result)) { raise(&ctx, "Failure at %u %s\n", __LINE__, __FILE__); }

#define BAIL_ON_BAD_RESULT(result) \
    TRY(&ctx){ \
        RAISE_ON_BAD_RESULT(result) \
    } CATCH {\
        fprintf(stderr, "%s", ctx.msg); \
        free(ctx.msg); \
        exit(EXIT_FAILURE); \
    }

uint32_t vkGetBestComputeQueueIndex(Ctx *context,
    VkPhysicalDevice physicalDevice);

uint32_t vkGetBestMemoryTypeIndex(Ctx *context,
    const VkPhysicalDeviceMemoryProperties *properties,
    VkMemoryPropertyFlags flags, size_t size);

VkPhysicalDevice vkGetPhysicalDevice_IGPOrDefault(
	VkInstance instance
);

#endif
