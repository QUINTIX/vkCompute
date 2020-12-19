#include "findForInit.h"
#include <stdbool.h>

uint32_t vkGetBestComputeQueueIndex(Ctx *context, VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyPropertiesCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = 
        (VkQueueFamilyProperties*)_alloca(
            sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
            &queueFamilyPropertiesCount, queueFamilyProperties);

    for(    uint32_t queueFamilyIndex=0; 
            queueFamilyIndex < queueFamilyPropertiesCount; 
            queueFamilyIndex++) {
        
        const VkQueueFlags actualFlags = 
                queueFamilyProperties[queueFamilyIndex].queueFlags;
        const VkQueueFlags maskedFlags = 
             ~VK_QUEUE_SPARSE_BINDING_BIT & actualFlags;
        
        if(!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && 
                (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
            return queueFamilyIndex;
        }
    }

    for(    uint32_t queueFamilyIndex=0; 
            queueFamilyIndex < queueFamilyPropertiesCount; 
            queueFamilyIndex++) {
        
        const VkQueueFlags actualFlags = 
                queueFamilyProperties[queueFamilyIndex].queueFlags;
        const VkQueueFlags maskedFlags = 
             ~VK_QUEUE_SPARSE_BINDING_BIT & actualFlags;

        if ((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) 
                & maskedFlags) {
            
            return queueFamilyIndex;
        }
    }

    raise(context, "no queue family found out of %u queues", 
            queueFamilyPropertiesCount);

    return -1; //unreachable
}

uint32_t vkGetBestMemoryTypeIndex(Ctx *context,
        const VkPhysicalDeviceMemoryProperties *properties,
        VkMemoryPropertyFlags flags, size_t size) {
    
    for(uint32_t i = 0; i < properties->memoryTypeCount; i++){
        VkMemoryType memoryType = (*properties).memoryTypes[i];
        VkMemoryHeap memoryHeap = (*properties).memoryHeaps[
            memoryType.heapIndex];
        bool isRightKind = !!(flags & memoryType.propertyFlags);
        bool isRightSize = !!(size < memoryHeap.size);
        if (isRightKind && isRightSize){
            return i;
        }
    }
    raise(context, "could not find matching memory type out of %u properties",
            properties->memoryTypeCount);
    return -1; //unreachable
}

VkPhysicalDevice vkGetPhysicalDevice_IGPOrDefault(
        VkInstance instance) {
    VkPhysicalDevice device;

    uint32_t physicalDeviceCount = 0;
    RAISE_ON_BAD_RESULT(
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL)
    );

    if(1 == physicalDeviceCount){
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &device);
        return device;
    }

        VkPhysicalDevice* const physicalDevices = (VkPhysicalDevice*)malloc(
        sizeof(VkPhysicalDevice) * physicalDeviceCount
    );

    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

    VkPhysicalDeviceProperties deviceProperties;
    for(uint32_t i = 0; i < physicalDeviceCount; i++){
        device = physicalDevices[i];
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU == deviceProperties
                    .deviceType){
            break;
        }
    };

    return device;
}
