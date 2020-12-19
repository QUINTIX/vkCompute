#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stddef.h>
#include "tryCatch.h"

#include "grabShaderModule.h"

VkShaderModule loadFromSPVFile(
        Ctx* context,
        VkDevice logicalDevice,
        VkShaderModuleCreateFlags flags,
        const char* fileName){

    VkShaderModule shader_module;

    FILE* shaderFile = fopen(fileName, "rb");
        fseek(shaderFile, 0, SEEK_END);
        const size_t shaderSize = (size_t)ftell(shaderFile);
        rewind(shaderFile);
        void* shader = (void*)_alloca(shaderSize);
        fread(shader, shaderSize, 1, shaderFile);
    fclose(shaderFile);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL, flags, shaderSize, shader
    };

    if(VK_SUCCESS != vkCreateShaderModule(
            logicalDevice, &shaderModuleCreateInfo, NULL, &shader_module)
    ){
        raise(context, "unable to load shader module %s", fileName);
    }

    return shader_module;
}
