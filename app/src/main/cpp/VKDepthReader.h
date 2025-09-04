//
// Created by benyuan on 2025/9/4.
//
#include <vulkan/vulkan_android.h>
#include <cinttypes>

#pragma once

#include <vulkan/vulkan.h>

class VKDepthReader
{
public:
    void Capture(VkImage img, std::uint32_t width, std::uint32_t height, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool cmdPool, VkQueue graphQueue);

};

