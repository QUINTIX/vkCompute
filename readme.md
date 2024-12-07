# Hello Vulkan Compute

[The top search result](https://gist.github.com/Overv/7ac07356037592a121225172d7d78f2d) 
for "hello vulkan triangle gist" has over 1000 source lines of code.

```text
[redacted]@[redacted]-MBP stash % sloc HelloTriangle.cc 

---------- Result ------------

            Physical :  1379
              Source :  1015
             Comment :  104
 Single-line comment :  104
       Block comment :  0
               Mixed :  1
 Empty block comment :  0
               Empty :  261
               To Do :  0

Number of files read :  1

----------------------------
```

Snooping around the vulkan docs gave me the suspicion that running a simple compute shader would be a much ligher lift. Turns out my suspicion was correct. Neil Henning (@sheredom) [has an excelent explainer](https://www.duskborn.com/posts/a-simple-vulkan-compute-example/) for a [single-file vulkan sample](https://www.duskborn.com/posts/a-simple-vulkan-compute-example/) he wrote that's nearly half that size.

```text
[redacted]@[redacted]-MBP old % sloc main.c 

---------- Result ------------

            Physical :  620
              Source :  445
             Comment :  52
 Single-line comment :  52
       Block comment :  0
               Mixed :  5
 Empty block comment :  0
               Empty :  128
               To Do :  0

Number of files read :  1

----------------------------
```

And that's despite having [embedded SPIR-V written in assembly](https://gist.github.com/sheredom/523f02bbad2ae397d7ed255f3f3b5a7f#file-vkcomputesample-L310), using an array of integers. Impressive, but I have some stylistic quibles with it, and I wanted to compile my own GLSL compute shader from source. So I did some refactoring.

```text
[redacted]@[redacted]-MBP vkcompute % sloc -a comp=c *.c *.h shader.comp

---------- Result ------------

            Physical :  660
              Source :  507
             Comment :  39
 Single-line comment :  30
       Block comment :  9
               Mixed :  3
 Empty block comment :  0
               Empty :  117
               To Do :  0

Number of files read :  8

----------------------------
```

A little bigger, but hopefully quite a bit clearer.

Major changes include
* I'm not a fan of outvars in place of return values, much as the vulkan library follows the pattern of returning status and overwritting an outvar. So I incorporated [a simple try/catch harness](https://gist.github.com/mori0091/45b275f61ac802fcabe7fb5dede7ca73) by @mori0091 */

```c
TRY(&ctx){
    VkPhysicalDevice physicalDevice = vkGetPhysicalDevice_IGPOrDefault(instance);
    doStuffOnDevice(&ctx, physicalDevice);
} CATCH {
    fprintf(stderr, "%s\n", ctx.msg);
    free((void*)ctx.msg);
    exit(EXIT_FAILURE);
}
```
* Instead of running the compute kernel on every device, as the name `vkGetPhysicalDevice_IGPOrDefault` suggests, it selects either an IGP or the last device listed, and only runs the compute shader on it
* I tried to minimize the number of magic numbers, using type cast `NULL`s in structs where Neil simply passed in zero
```c
/*Neil's code*/
BAIL_ON_BAD_RESULT(vkCreateInstance(&instanceCreateInfo, 0, &instance));

/*My change*/
BAIL_ON_BAD_RESULT(vkCreateInstance(
        &instanceCreateInfo, (const VkAllocationCallbacks*) NULL, &instance));
```
* Wherever in vulkan structs the pattern `VkStructureType sType; const void* pNext;` is used, instead of an enum value followed by zero, I put the enum value and NULL on the same line.
* Wherever in vulkan structs the pattern `uint32_t count; VkThingBeingCounted* canBeMoreThanOne;` is used, I put the count and the thing being counted on the same line, using type cast nulls whenever the count is 0 in place of a mystery pair of zeros.
```c
/*Neil's*/
VkSubmitInfo submitInfo = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    0,
    0,
    0,
    0,
    1,
    &commandBuffer,
    0,
    0
};

/*Mine*/
VkSubmitInfo submitInfo = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL,
    0, (const VkSemaphore*)NULL,
    (const VkPipelineStageFlags*)NULL,
    1, &commandBuffer,
    0, (const VkSemaphore*)NULL
};
```

