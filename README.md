# Vulkan starter

<!-- TOC -->

- [Vulkan starter](#vulkan-starter)
    - [Dependencies](#dependencies)
    - [Known issues](#known-issues)
        - [M15 V104 'destinationstage' used without being initialized](#m15-v104-destinationstage-used-without-being-initialized)
        - [M15 V104 depth-only image formats VKIMAGEASPECTDEPTHBIT](#m15-v104-depth-only-image-formats-vkimageaspectdepthbit)
    - [Learning notes](#learning-notes)

<!-- /TOC -->

<img src="https://pbs.twimg.com/media/GioHlduXoAACmxc?format=jpg&name=medium" alt="Vulkan meme" width="300">

Learning how Vulkan API is usually used, and maybe have some idea of its architecture.

Made following [Graphics Programming with Vulkan and C++](https://www.udemy.com/course/graphics-programming-with-vulkan-and-cpp/).

The course used `VulkanSDK-1.3.261` at the time I watched. I used a newer version `VulkanSDK-1.3.290`. This caused a little bit of trouble in the last module but no big deal.

## Dependencies

- VulkanSDK-1.3.290
- Cmake 3.27.4
- Ninja 1.12.1
- Visual Studio Community 2022

## Known issues

To shorten the titles in this section, using M=Module, V=Video to abbreviate. For example M15 V104 means video 104 in module 15 - Advanced.

### M15 V104 'destination_stage' used without being initialized

The `destination_stage` in `TransitionImageLayout()` was un-initilized before these if blocks:

```c++
...
        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
...
```

This did not cause any problem in the video. I guess it was because of using different VulkanSDK versions, or the instructor initialized them off screen.

Anyway, if leaving them uninitizalized, this error would occur:

```
Run-Time Check Failure #3 - The variable 'destination_stage' is being used without being initialized.
```

So after adding initialization before the if blocks:

```c++
VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // initilize

if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
    new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
...
```

The error was gone. This is the classic uninitilized variable error quirk in C++.

### M15 V104 depth-only image formats VK_IMAGE_ASPECT_DEPTH_BIT

If following the video closely, the final compile still failed in my case. The error was:

```
[2025-08-08 22:13:34.457] [error] Vulkan Error: Validation Error: [ UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect ] Object 0: handle = 0x3a6cbb0000000025, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0x90ef715d | vkCmdPipelineBarrier: Using format (VK_FORMAT_D32_SFLOAT) with aspect flags (VK_IMAGE_ASPECT_COLOR_BIT) but depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.
```

This also originated from `TransitionImageLayout()`, where the instructor tried to set the aspectMask based on the layout, in order to avoid exposing an extra argument for image format:

```c++
if (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
}
else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
}
```

. I am not sure why this did not trigger errors for them but it did trigger error when I compiled it. Maybe this time it really was because of different VulkanSDK versions.

So the best fix is to expose an extra argument for image format:

```c++
void Graphics::TransitionImageLayout(
    VkImage image, 
    VkFormat format,    // extra argument
    VkImageLayout old_layout, 
    VkImageLayout new_layout) 
{
    ...
    // Set aspectMask based on format
    if (format == VK_FORMAT_D32_SFLOAT || 
        format == VK_FORMAT_D16_UNORM || 
        format == VK_FORMAT_D24_UNORM_S8_UINT || 
        format == VK_FORMAT_D32_SFLOAT_S8_UINT) 
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        // Add stencil bit if format has stencil
        if (format == VK_FORMAT_D24_UNORM_S8_UINT || 
            format == VK_FORMAT_D32_SFLOAT_S8_UINT) 
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } 
    else 
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    ...
```

At least it worked for me at the time. And then I also needed to change how other functions call it, but that was trivial.


## Learning notes

There are some notes while learning, re-formatted with the help of Claude.ai, chatgpt.com, etc.

[NOTES.md](./NOTES.md)
