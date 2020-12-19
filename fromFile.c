// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include <stdio.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

#include "tryCatch.h"
#include "findForInit.h"
#include "grabShaderModule.h"

// based on https://www.duskborn.com/posts/a-simple-vulkan-compute-example/
void doStuffOnDevice(Ctx *context_, VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyIndex = vkGetBestComputeQueueIndex(context_, 
                    physicalDevice);
    printf("found compatible queue at index %u\n", queueFamilyIndex);

    const float queuePrioritory = 1.0f;
    const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL,
        (VkDeviceQueueCreateFlags)0,
        queueFamilyIndex,
        1,
        &queuePrioritory
    };

    const VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, NULL,
        (VkDeviceCreateFlags)0,
        1, &deviceQueueCreateInfo,
        0, (const char* const*)NULL,
        0, (const char* const*)NULL,
        (const VkPhysicalDeviceFeatures*) NULL
    };

    VkDevice logicalDevice;

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, 0,
        &logicalDevice); RAISE_ON_BAD_RESULT(result);

    VkPhysicalDeviceMemoryProperties properties;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

    const int32_t bufferLength = 16384;
    const uint32_t bufferSize = sizeof(float) * bufferLength;

    // we are going to need two buffers from this one memory
    const VkDeviceSize memorySize = bufferSize * 2; 

    uint32_t memoryTypeIndex = vkGetBestMemoryTypeIndex(
        &ctx, &properties, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        (size_t)bufferSize );
    printf("found memory slot at index %u\n", memoryTypeIndex);

    const VkMemoryAllocateInfo memoryAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL,
        memorySize,
        memoryTypeIndex
    };

    VkDeviceMemory memory;
    RAISE_ON_BAD_RESULT(vkAllocateMemory(logicalDevice,
            &memoryAllocateInfo, (VkAllocationCallbacks*)NULL, &memory));

    float *payload;
    RAISE_ON_BAD_RESULT(vkMapMemory(logicalDevice, 
            memory, 0, memorySize, (VkMemoryMapFlags)0, (void *)&payload));

    for(uint32_t k = 0; k < memorySize / sizeof(float); k++){
        payload[k] = (float)k * 0.5f;
    }

    vkUnmapMemory(logicalDevice, memory);

    const VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL,
        (VkBufferCreateFlags)0,
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        1, &queueFamilyIndex
    };

    VkBuffer in_buffer;
    RAISE_ON_BAD_RESULT(vkCreateBuffer(logicalDevice,
            &bufferCreateInfo, (const VkAllocationCallbacks*)NULL, &in_buffer));

    RAISE_ON_BAD_RESULT(vkBindBufferMemory(logicalDevice,
            in_buffer, memory, 0));

    VkBuffer out_buffer;
    RAISE_ON_BAD_RESULT(vkCreateBuffer(logicalDevice,
            &bufferCreateInfo, (const VkAllocationCallbacks*)NULL, &out_buffer));

    RAISE_ON_BAD_RESULT(vkBindBufferMemory(logicalDevice,
            out_buffer, memory, bufferSize));

    VkShaderModule shader_module = loadFromSPVFile(&ctx, logicalDevice,
            (VkShaderModuleCreateFlags)0, "shader.comp.spv");
    
    #define NUM_BINDINGS 2
    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[NUM_BINDINGS] = {
      {
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        (const VkSampler*)NULL
      },
      {
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        (const VkSampler*)NULL
      }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL,
        (VkDescriptorSetLayoutCreateFlags) 0,
        NUM_BINDINGS, descriptorSetLayoutBindings
    };

    VkDescriptorSetLayout descriptorSetLayout;
    RAISE_ON_BAD_RESULT(vkCreateDescriptorSetLayout(logicalDevice, 
            &descriptorSetLayoutCreateInfo, (const VkAllocationCallbacks*)NULL, 
            &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, NULL,
        (VkPipelineLayoutCreateFlags) 0,
        1, &descriptorSetLayout,
        0, (const VkPushConstantRange*) NULL
    };

    VkPipelineLayout pipelineLayout;
    RAISE_ON_BAD_RESULT(vkCreatePipelineLayout(logicalDevice,
            &pipelineLayoutCreateInfo, (const VkAllocationCallbacks*) NULL, &pipelineLayout));

    VkComputePipelineCreateInfo computePipelineCreateInfo = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, NULL,
      (VkPipelineCreateFlags) 0,
      {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL,
        (VkPipelineShaderStageCreateFlags) 0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        shader_module,
        "main",
        (const VkSpecializationInfo*) NULL
      },
      pipelineLayout,
      (VkPipeline) NULL,
      0
    };

    VkPipeline pipeline;
    RAISE_ON_BAD_RESULT(vkCreateComputePipelines(logicalDevice, 
        (VkPipelineCache) NULL,
        1, &computePipelineCreateInfo, 
        (const VkAllocationCallbacks*)NULL, &pipeline));

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, NULL,
      (VkCommandPoolCreateFlags) 0,
      queueFamilyIndex
    };

    VkDescriptorPoolSize descriptorPoolSize = {
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      2
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, NULL,
      (VkDescriptorPoolCreateFlags) 0,
      1,
      1, &descriptorPoolSize
    };

    VkDescriptorPool descriptorPool;
    RAISE_ON_BAD_RESULT(vkCreateDescriptorPool(logicalDevice,
            &descriptorPoolCreateInfo, (const VkAllocationCallbacks*)NULL,
            &descriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL,
      descriptorPool,
      1, &descriptorSetLayout
    };

    VkDescriptorSet descriptorSet;
    RAISE_ON_BAD_RESULT(vkAllocateDescriptorSets(logicalDevice,
            &descriptorSetAllocateInfo, &descriptorSet));

    VkDescriptorBufferInfo in_descriptorBufferInfo = {
      in_buffer,
      0,
      VK_WHOLE_SIZE
    };

    VkDescriptorBufferInfo out_descriptorBufferInfo = {
      out_buffer,
      0,
      VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet writeDescriptorSet[2] = {
      {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        (const VkDescriptorImageInfo*) NULL,
        &in_descriptorBufferInfo,
        (const VkBufferView*) NULL
      },
      {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
        descriptorSet,
        1,
        0,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        (const VkDescriptorImageInfo*) NULL,
        &out_descriptorBufferInfo,
        (const VkBufferView*) NULL
      }
    };

    vkUpdateDescriptorSets(logicalDevice, 
        2, writeDescriptorSet, 
        0, (const VkCopyDescriptorSet*) NULL);

    VkCommandPool commandPool;
    RAISE_ON_BAD_RESULT(vkCreateCommandPool(logicalDevice,
            &commandPoolCreateInfo, (const VkAllocationCallbacks*) NULL,
            &commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL,
      commandPool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      1
    };

    VkCommandBuffer commandBuffer;
    RAISE_ON_BAD_RESULT(vkAllocateCommandBuffers(logicalDevice,
            &commandBufferAllocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      (const VkCommandBufferInheritanceInfo*)NULL
    };

    RAISE_ON_BAD_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      pipelineLayout, 0,
      1, &descriptorSet, 
      0, (const uint32_t*) NULL);

    vkCmdDispatch(commandBuffer, bufferSize / sizeof(float), 1, 1);

    RAISE_ON_BAD_RESULT(vkEndCommandBuffer(commandBuffer));

    VkQueue queue;
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, 0, &queue);

    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL,
        0, (const VkSemaphore*)NULL,
        (const VkPipelineStageFlags*)NULL,
        1, &commandBuffer,
        0, (const VkSemaphore*)NULL
    };

    RAISE_ON_BAD_RESULT(vkQueueSubmit(queue, 1, &submitInfo, 
        (VkFence)NULL));

    RAISE_ON_BAD_RESULT(vkQueueWaitIdle(queue));

    RAISE_ON_BAD_RESULT(vkMapMemory(logicalDevice,
            memory, 0, memorySize, (VkMemoryMapFlags)0, (void *)&payload));

    const uint32_t numfloats = bufferSize / sizeof(float);
    const float* inputBuffer = payload;
    const float* outputBuffer = payload + numfloats;

    printf("input buffer\n");
    for(uint32_t k=0; k < numfloats; k++){
        printf("%07.1f ", inputBuffer[k]);
        if((k & 0x1F) == 0x1F){
            printf("\n");
        }
    }
    printf("\noutput buffer\n");
    for(uint32_t k=0; k < numfloats; k++){
        printf("%07.1f ", outputBuffer[k]);
        if((k & 0x1F) == 0x1F){
            printf("\n");
        }
    }
    
}

int main(int argc, const char * const argv[]) {
    (void)argc;
    (void)argv;

    const VkApplicationInfo applicationInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL,
        "VKFromFileComputeSample",
        0,
        "",
        0,
        VK_MAKE_VERSION(1, 0, 9)
    };

    const VkInstanceCreateInfo instanceCreateInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL,
        (VkInstanceCreateFlags) 0, 
        &applicationInfo, 
        0, (const char* const*)NULL, 
        0, (const char* const*)NULL
    };

    VkInstance instance;
    TRY(&ctx){
        RAISE_ON_BAD_RESULT(
            vkCreateInstance(&instanceCreateInfo, (const VkAllocationCallbacks*) NULL, 
            &instance)
        );
    } CATCH {
        fprintf(stderr, "%s\n", ctx.msg);
        free((void*)ctx.msg);
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice physicalDevice = vkGetPhysicalDevice_IGPOrDefault(instance);

    TRY(&ctx){
        doStuffOnDevice(&ctx, physicalDevice);
    } CATCH {
        fprintf(stderr, "%s\n", ctx.msg);
        free((void*)ctx.msg);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
