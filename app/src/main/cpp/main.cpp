#include <jni.h>
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <vector>
#include <array>
#include <cstring>
#include <cassert>
extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>
}



#define VK_CHECK(x) do { VkResult err = (x); if (err) { LOGE("Vulkan error %d at %s:%d", err, __FILE__, __LINE__); abort(); } } while(0)

// 需要替换为你自己离线编译得到的 SPIR-V 字节码
constexpr const uint32_t SPV_offscreen_vert[] = {
        0x07230203, 0x00010000, 0x000D000B, 0x00000036, 0x00000000, 0x00020011, 0x00000001, 0x0006000B,
        0x00000001, 0x4C534C47, 0x6474732E, 0x3035342E, 0x00000000, 0x0003000E, 0x00000000, 0x00000001,
        0x0008000F, 0x00000000, 0x00000004, 0x6E69616D, 0x00000000, 0x00000022, 0x00000026, 0x00000031,
        0x00030047, 0x00000020, 0x00000002, 0x00050048, 0x00000020, 0x00000000, 0x0000000B, 0x00000000,
        0x00050048, 0x00000020, 0x00000001, 0x0000000B, 0x00000001, 0x00050048, 0x00000020, 0x00000002,
        0x0000000B, 0x00000003, 0x00050048, 0x00000020, 0x00000003, 0x0000000B, 0x00000004, 0x00040047,
        0x00000026, 0x0000000B, 0x0000002A, 0x00040047, 0x00000031, 0x0000001E, 0x00000000, 0x00020013,
        0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
        0x00000007, 0x00000006, 0x00000002, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002B,
        0x00000008, 0x00000009, 0x00000003, 0x0004001C, 0x0000000A, 0x00000007, 0x00000009, 0x00040020,
        0x0000000B, 0x00000007, 0x0000000A, 0x0004002B, 0x00000006, 0x0000000D, 0x00000000, 0x0004002B,
        0x00000006, 0x0000000E, 0xBF19999A, 0x0005002C, 0x00000007, 0x0000000F, 0x0000000D, 0x0000000E,
        0x0004002B, 0x00000006, 0x00000010, 0x3F19999A, 0x0005002C, 0x00000007, 0x00000011, 0x00000010,
        0x00000010, 0x0005002C, 0x00000007, 0x00000012, 0x0000000E, 0x00000010, 0x0006002C, 0x0000000A,
        0x00000013, 0x0000000F, 0x00000011, 0x00000012, 0x00040017, 0x00000014, 0x00000006, 0x00000003,
        0x0004001C, 0x00000015, 0x00000014, 0x00000009, 0x00040020, 0x00000016, 0x00000007, 0x00000015,
        0x0004002B, 0x00000006, 0x00000018, 0x3F800000, 0x0006002C, 0x00000014, 0x00000019, 0x00000018,
        0x0000000D, 0x0000000D, 0x0006002C, 0x00000014, 0x0000001A, 0x0000000D, 0x00000018, 0x0000000D,
        0x0006002C, 0x00000014, 0x0000001B, 0x0000000D, 0x0000000D, 0x00000018, 0x0006002C, 0x00000015,
        0x0000001C, 0x00000019, 0x0000001A, 0x0000001B, 0x00040017, 0x0000001D, 0x00000006, 0x00000004,
        0x0004002B, 0x00000008, 0x0000001E, 0x00000001, 0x0004001C, 0x0000001F, 0x00000006, 0x0000001E,
        0x0006001E, 0x00000020, 0x0000001D, 0x00000006, 0x0000001F, 0x0000001F, 0x00040020, 0x00000021,
        0x00000003, 0x00000020, 0x0004003B, 0x00000021, 0x00000022, 0x00000003, 0x00040015, 0x00000023,
        0x00000020, 0x00000001, 0x0004002B, 0x00000023, 0x00000024, 0x00000000, 0x00040020, 0x00000025,
        0x00000001, 0x00000023, 0x0004003B, 0x00000025, 0x00000026, 0x00000001, 0x00040020, 0x00000028,
        0x00000007, 0x00000007, 0x00040020, 0x0000002E, 0x00000003, 0x0000001D, 0x00040020, 0x00000030,
        0x00000003, 0x00000014, 0x0004003B, 0x00000030, 0x00000031, 0x00000003, 0x00040020, 0x00000033,
        0x00000007, 0x00000014, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200F8,
        0x00000005, 0x0004003B, 0x0000000B, 0x0000000C, 0x00000007, 0x0004003B, 0x00000016, 0x00000017,
        0x00000007, 0x0003003E, 0x0000000C, 0x00000013, 0x0003003E, 0x00000017, 0x0000001C, 0x0004003D,
        0x00000023, 0x00000027, 0x00000026, 0x00050041, 0x00000028, 0x00000029, 0x0000000C, 0x00000027,
        0x0004003D, 0x00000007, 0x0000002A, 0x00000029, 0x00050051, 0x00000006, 0x0000002B, 0x0000002A,
        0x00000000, 0x00050051, 0x00000006, 0x0000002C, 0x0000002A, 0x00000001, 0x00070050, 0x0000001D,
        0x0000002D, 0x0000002B, 0x0000002C, 0x0000000D, 0x00000018, 0x00050041, 0x0000002E, 0x0000002F,
        0x00000022, 0x00000024, 0x0003003E, 0x0000002F, 0x0000002D, 0x00050041, 0x00000033, 0x00000034,
        0x00000017, 0x00000027, 0x0004003D, 0x00000014, 0x00000035, 0x00000034, 0x0003003E, 0x00000031,
        0x00000035, 0x000100FD, 0x00010038
};
constexpr const size_t SPV_offscreen_vert_size = sizeof(SPV_offscreen_vert);

constexpr const uint32_t SPV_offscreen_frag[] = {
        0x07230203, 0x00010000, 0x000D000B, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x0006000B,
        0x00000001, 0x4C534C47, 0x6474732E, 0x3035342E, 0x00000000, 0x0003000E, 0x00000000, 0x00000001,
        0x0007000F, 0x00000004, 0x00000004, 0x6E69616D, 0x00000000, 0x00000009, 0x0000000C, 0x00030010,
        0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001C2, 0x000A0004, 0x475F4C47, 0x4C474F4F,
        0x70635F45, 0x74735F70, 0x5F656C79, 0x656E696C, 0x7269645F, 0x69746365, 0x00006576, 0x00080004,
        0x475F4C47, 0x4C474F4F, 0x6E695F45, 0x64756C63, 0x69645F65, 0x74636572, 0x00657669, 0x00040005,
        0x00000004, 0x6E69616D, 0x00000000, 0x00050005, 0x00000009, 0x4374756F, 0x726F6C6F, 0x00000000,
        0x00040005, 0x0000000C, 0x6C6F4376, 0x0000726F, 0x00040047, 0x00000009, 0x0000001E, 0x00000000,
        0x00040047, 0x0000000C, 0x0000001E, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
        0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003B, 0x00000008, 0x00000009, 0x00000003,
        0x00040017, 0x0000000A, 0x00000006, 0x00000003, 0x00040020, 0x0000000B, 0x00000001, 0x0000000A,
        0x0004003B, 0x0000000B, 0x0000000C, 0x00000001, 0x0004002B, 0x00000006, 0x0000000E, 0x3F800000,
        0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200F8, 0x00000005, 0x0004003D,
        0x0000000A, 0x0000000D, 0x0000000C, 0x00050051, 0x00000006, 0x0000000F, 0x0000000D, 0x00000000,
        0x00050051, 0x00000006, 0x00000010, 0x0000000D, 0x00000001, 0x00050051, 0x00000006, 0x00000011,
        0x0000000D, 0x00000002, 0x00070050, 0x00000007, 0x00000012, 0x0000000F, 0x00000010, 0x00000011,
        0x0000000E, 0x0003003E, 0x00000009, 0x00000012, 0x000100FD, 0x00010038
};
constexpr const size_t SPV_offscreen_frag_size = sizeof(SPV_offscreen_frag);

constexpr const uint32_t SPV_present_vert[] = {
        0x07230203, 0x00010000, 0x000D000B, 0x00000033, 0x00000000, 0x00020011, 0x00000001, 0x0006000B,
        0x00000001, 0x4C534C47, 0x6474732E, 0x3035342E, 0x00000000, 0x0003000E, 0x00000000, 0x00000001,
        0x0008000F, 0x00000000, 0x00000004, 0x6E69616D, 0x00000000, 0x0000001F, 0x00000023, 0x0000002F,
        0x00030003, 0x00000002, 0x000001C2, 0x000A0004, 0x475F4C47, 0x4C474F4F, 0x70635F45, 0x74735F70,
        0x5F656C79, 0x656E696C, 0x7269645F, 0x69746365, 0x00006576, 0x00080004, 0x475F4C47, 0x4C474F4F,
        0x6E695F45, 0x64756C63, 0x69645F65, 0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6E69616D,
        0x00000000, 0x00030005, 0x0000000C, 0x00736F70, 0x00030005, 0x00000013, 0x00007675, 0x00060005,
        0x0000001D, 0x505F6C67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000001D, 0x00000000,
        0x505F6C67, 0x7469736F, 0x006E6F69, 0x00070006, 0x0000001D, 0x00000001, 0x505F6C67, 0x746E696F,
        0x657A6953, 0x00000000, 0x00070006, 0x0000001D, 0x00000002, 0x435F6C67, 0x4470696C, 0x61747369,
        0x0065636E, 0x00070006, 0x0000001D, 0x00000003, 0x435F6C67, 0x446C6C75, 0x61747369, 0x0065636E,
        0x00030005, 0x0000001F, 0x00000000, 0x00060005, 0x00000023, 0x565F6C67, 0x65747265, 0x646E4978,
        0x00007865, 0x00030005, 0x0000002F, 0x00565576, 0x00030047, 0x0000001D, 0x00000002, 0x00050048,
        0x0000001D, 0x00000000, 0x0000000B, 0x00000000, 0x00050048, 0x0000001D, 0x00000001, 0x0000000B,
        0x00000001, 0x00050048, 0x0000001D, 0x00000002, 0x0000000B, 0x00000003, 0x00050048, 0x0000001D,
        0x00000003, 0x0000000B, 0x00000004, 0x00040047, 0x00000023, 0x0000000B, 0x0000002A, 0x00040047,
        0x0000002F, 0x0000001E, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
        0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000002, 0x00040015,
        0x00000008, 0x00000020, 0x00000000, 0x0004002B, 0x00000008, 0x00000009, 0x00000003, 0x0004001C,
        0x0000000A, 0x00000007, 0x00000009, 0x00040020, 0x0000000B, 0x00000007, 0x0000000A, 0x0004002B,
        0x00000006, 0x0000000D, 0xBF800000, 0x0005002C, 0x00000007, 0x0000000E, 0x0000000D, 0x0000000D,
        0x0004002B, 0x00000006, 0x0000000F, 0x40400000, 0x0005002C, 0x00000007, 0x00000010, 0x0000000F,
        0x0000000D, 0x0005002C, 0x00000007, 0x00000011, 0x0000000D, 0x0000000F, 0x0006002C, 0x0000000A,
        0x00000012, 0x0000000E, 0x00000010, 0x00000011, 0x0004002B, 0x00000006, 0x00000014, 0x00000000,
        0x0005002C, 0x00000007, 0x00000015, 0x00000014, 0x00000014, 0x0004002B, 0x00000006, 0x00000016,
        0x40000000, 0x0005002C, 0x00000007, 0x00000017, 0x00000016, 0x00000014, 0x0005002C, 0x00000007,
        0x00000018, 0x00000014, 0x00000016, 0x0006002C, 0x0000000A, 0x00000019, 0x00000015, 0x00000017,
        0x00000018, 0x00040017, 0x0000001A, 0x00000006, 0x00000004, 0x0004002B, 0x00000008, 0x0000001B,
        0x00000001, 0x0004001C, 0x0000001C, 0x00000006, 0x0000001B, 0x0006001E, 0x0000001D, 0x0000001A,
        0x00000006, 0x0000001C, 0x0000001C, 0x00040020, 0x0000001E, 0x00000003, 0x0000001D, 0x0004003B,
        0x0000001E, 0x0000001F, 0x00000003, 0x00040015, 0x00000020, 0x00000020, 0x00000001, 0x0004002B,
        0x00000020, 0x00000021, 0x00000000, 0x00040020, 0x00000022, 0x00000001, 0x00000020, 0x0004003B,
        0x00000022, 0x00000023, 0x00000001, 0x00040020, 0x00000025, 0x00000007, 0x00000007, 0x0004002B,
        0x00000006, 0x00000028, 0x3F800000, 0x00040020, 0x0000002C, 0x00000003, 0x0000001A, 0x00040020,
        0x0000002E, 0x00000003, 0x00000007, 0x0004003B, 0x0000002E, 0x0000002F, 0x00000003, 0x00050036,
        0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200F8, 0x00000005, 0x0004003B, 0x0000000B,
        0x0000000C, 0x00000007, 0x0004003B, 0x0000000B, 0x00000013, 0x00000007, 0x0003003E, 0x0000000C,
        0x00000012, 0x0003003E, 0x00000013, 0x00000019, 0x0004003D, 0x00000020, 0x00000024, 0x00000023,
        0x00050041, 0x00000025, 0x00000026, 0x0000000C, 0x00000024, 0x0004003D, 0x00000007, 0x00000027,
        0x00000026, 0x00050051, 0x00000006, 0x00000029, 0x00000027, 0x00000000, 0x00050051, 0x00000006,
        0x0000002A, 0x00000027, 0x00000001, 0x00070050, 0x0000001A, 0x0000002B, 0x00000029, 0x0000002A,
        0x00000014, 0x00000028, 0x00050041, 0x0000002C, 0x0000002D, 0x0000001F, 0x00000021, 0x0003003E,
        0x0000002D, 0x0000002B, 0x0004003D, 0x00000020, 0x00000030, 0x00000023, 0x00050041, 0x00000025,
        0x00000031, 0x00000013, 0x00000030, 0x0004003D, 0x00000007, 0x00000032, 0x00000031, 0x0003003E,
        0x0000002F, 0x00000032, 0x000100FD, 0x00010038
};
constexpr const size_t SPV_present_vert_size = sizeof(SPV_present_vert);

constexpr const uint32_t SPV_present_frag[] = {
        0x07230203, 0x00010000, 0x000D000B, 0x00000014, 0x00000000, 0x00020011, 0x00000001, 0x0006000B,
        0x00000001, 0x4C534C47, 0x6474732E, 0x3035342E, 0x00000000, 0x0003000E, 0x00000000, 0x00000001,
        0x0007000F, 0x00000004, 0x00000004, 0x6E69616D, 0x00000000, 0x00000009, 0x00000011, 0x00030010,
        0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001C2, 0x000A0004, 0x475F4C47, 0x4C474F4F,
        0x70635F45, 0x74735F70, 0x5F656C79, 0x656E696C, 0x7269645F, 0x69746365, 0x00006576, 0x00080004,
        0x475F4C47, 0x4C474F4F, 0x6E695F45, 0x64756C63, 0x69645F65, 0x74636572, 0x00657669, 0x00040005,
        0x00000004, 0x6E69616D, 0x00000000, 0x00050005, 0x00000009, 0x4374756F, 0x726F6C6F, 0x00000000,
        0x00040005, 0x0000000D, 0x30786574, 0x00000000, 0x00030005, 0x00000011, 0x00565576, 0x00040047,
        0x00000009, 0x0000001E, 0x00000000, 0x00040047, 0x0000000D, 0x00000021, 0x00000000, 0x00040047,
        0x0000000D, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x0000001E, 0x00000000, 0x00020013,
        0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
        0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003B,
        0x00000008, 0x00000009, 0x00000003, 0x00090019, 0x0000000A, 0x00000006, 0x00000001, 0x00000000,
        0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001B, 0x0000000B, 0x0000000A, 0x00040020,
        0x0000000C, 0x00000000, 0x0000000B, 0x0004003B, 0x0000000C, 0x0000000D, 0x00000000, 0x00040017,
        0x0000000F, 0x00000006, 0x00000002, 0x00040020, 0x00000010, 0x00000001, 0x0000000F, 0x0004003B,
        0x00000010, 0x00000011, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
        0x000200F8, 0x00000005, 0x0004003D, 0x0000000B, 0x0000000E, 0x0000000D, 0x0004003D, 0x0000000F,
        0x00000012, 0x00000011, 0x00050057, 0x00000007, 0x00000013, 0x0000000E, 0x00000012, 0x0003003E,
        0x00000009, 0x00000013, 0x000100FD, 0x00010038
};
constexpr const size_t SPV_present_frag_size = sizeof(SPV_present_frag);

struct VulkanApp {
    ANativeWindow* window = nullptr;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t gfxQueueFamily = 0;
    VkQueue gfxQueue = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent{};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkRenderPass onscreenRenderPass = VK_NULL_HANDLE;

// Offscreen (texture) target
    VkFormat offscreenFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImage offscreenImage = VK_NULL_HANDLE;
    VkDeviceMemory offscreenMemory = VK_NULL_HANDLE;
    VkImageView offscreenView = VK_NULL_HANDLE;
    VkRenderPass offscreenRenderPass = VK_NULL_HANDLE;
    VkFramebuffer offscreenFramebuffer = VK_NULL_HANDLE;

// Descriptor/sampler for present pass
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

// Pipelines
    VkPipelineLayout pipelineLayoutOffscreen = VK_NULL_HANDLE;
    VkPipeline pipelineOffscreen = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayoutPresent = VK_NULL_HANDLE;
    VkPipeline pipelinePresent = VK_NULL_HANDLE;

// Command
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

// Sync
    VkSemaphore semImageAvailable = VK_NULL_HANDLE;
    VkSemaphore semRenderFinished = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;

// State
    bool running = false;
    bool firstFrame = true;
    VkImageLayout offscreenLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    void init(ANativeWindow* win) {
        window = win;
        createInstance();
        createSurface();
        pickDevice();
        createDevice();
        createSwapchain();
        createOnscreenRenderPass();
        createOffscreenTarget();
        createDescriptorsAndSampler();
        createPipelines();
        createFramebuffers();
        createCommandResources();
        running = true;
    }

    void shutdown() {
        if (device == VK_NULL_HANDLE) return;
        vkDeviceWaitIdle(device);

        vkDestroyFence(device, inFlight, nullptr);
        vkDestroySemaphore(device, semRenderFinished, nullptr);
        vkDestroySemaphore(device, semImageAvailable, nullptr);
        vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
        vkDestroyCommandPool(device, cmdPool, nullptr);

        for (auto fb : swapchainFramebuffers) vkDestroyFramebuffer(device, fb, nullptr);
        for (auto v : swapchainViews) vkDestroyImageView(device, v, nullptr);
        vkDestroyRenderPass(device, onscreenRenderPass, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);

        vkDestroyPipeline(device, pipelineOffscreen, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayoutOffscreen, nullptr);
        vkDestroyPipeline(device, pipelinePresent, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayoutPresent, nullptr);

        vkDestroyDescriptorPool(device, descPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
        vkDestroySampler(device, sampler, nullptr);

        vkDestroyFramebuffer(device, offscreenFramebuffer, nullptr);
        vkDestroyRenderPass(device, offscreenRenderPass, nullptr);
        vkDestroyImageView(device, offscreenView, nullptr);
        vkDestroyImage(device, offscreenImage, nullptr);
        vkFreeMemory(device, offscreenMemory, nullptr);

        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        device = VK_NULL_HANDLE;
        instance = VK_NULL_HANDLE;
    }

    void drawFrame() {
        // 1) Acquire swapchain image
        uint32_t imageIndex = 0;
        vkWaitForFences(device, 1, &inFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlight);

        VkResult acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semImageAvailable, VK_NULL_HANDLE, &imageIndex);
        if (acq == VK_ERROR_OUT_OF_DATE_KHR) {
            LOGI("Swapchain out of date (not handled).");
            return;
        }
        VK_CHECK(acq);

        // 2) Record command buffer (offscreen pass -> transition -> onscreen pass)
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

        // Transition offscreen image to COLOR_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcAccessMask = firstFrame ? 0 : VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = firstFrame ? VK_IMAGE_LAYOUT_UNDEFINED : offscreenLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = offscreenImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                    cmd,
                    firstFrame ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier
            );
        }

        // Offscreen render pass: draw colored triangle into offscreenImage
        {
            VkClearValue clear{};
            clear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rpbi.renderPass = offscreenRenderPass;
            rpbi.framebuffer = offscreenFramebuffer;
            rpbi.renderArea.offset = {0, 0};
            rpbi.renderArea.extent = swapchainExtent;
            rpbi.clearValueCount = 1;
            rpbi.pClearValues = &clear;

            vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport vp{};
            vp.x = 0; vp.y = 0;
            vp.width = (float)swapchainExtent.width;
            vp.height = (float)swapchainExtent.height;
            vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
            VkRect2D sc{{0,0}, swapchainExtent};
            vkCmdSetViewport(cmd, 0, 1, &vp);
            vkCmdSetScissor(cmd, 0, 1, &sc);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineOffscreen);
            vkCmdDraw(cmd, 3, 1, 0, 0);
            vkCmdEndRenderPass(cmd);
        }

        // Transition offscreen image to SHADER_READ_ONLY for sampling
        {
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = offscreenImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier
            );
            offscreenLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            firstFrame = false;
        }

        // Onscreen render pass: sample the offscreen texture and draw full-screen triangle
        {
            VkClearValue clear{};
            clear.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

            VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rpbi.renderPass = onscreenRenderPass;
            rpbi.framebuffer = swapchainFramebuffers[imageIndex];
            rpbi.renderArea.offset = {0, 0};
            rpbi.renderArea.extent = swapchainExtent;
            rpbi.clearValueCount = 1;
            rpbi.pClearValues = &clear;

            vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport vp{};
            vp.x = 0; vp.y = 0;
            vp.width = (float)swapchainExtent.width;
            vp.height = (float)swapchainExtent.height;
            vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
            VkRect2D sc{{0,0}, swapchainExtent};
            vkCmdSetViewport(cmd, 0, 1, &vp);
            vkCmdSetScissor(cmd, 0, 1, &sc);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinePresent);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutPresent, 0, 1, &descSet, 0, nullptr);
            vkCmdDraw(cmd, 3, 1, 0, 0);

            vkCmdEndRenderPass(cmd);
        }

        VK_CHECK(vkEndCommandBuffer(cmd));

        // 3) Submit and present
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &semImageAvailable;
        submit.pWaitDstStageMask = &waitStage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &semRenderFinished;

        VK_CHECK(vkQueueSubmit(gfxQueue, 1, &submit, inFlight));

        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &semRenderFinished;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &imageIndex;
        VkResult pr = vkQueuePresentKHR(gfxQueue, &present);
        if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR) {
            LOGI("Present out of date/suboptimal (not handled).");
            return;
        }
        VK_CHECK(pr);
    }

// ---- helpers ----

    void createInstance() {
        std::vector<const char*> layers;
        std::vector<const char*> exts = {
                "VK_KHR_surface",
                "VK_KHR_android_surface"
        };

        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "TriangleToTexture";
        appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.pEngineName = "NoEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ici.pApplicationInfo = &appInfo;
        ici.enabledLayerCount = (uint32_t)layers.size();
        ici.ppEnabledLayerNames = layers.empty()? nullptr: layers.data();
        ici.enabledExtensionCount = (uint32_t)exts.size();
        ici.ppEnabledExtensionNames = exts.data();

        VK_CHECK(vkCreateInstance(&ici, nullptr, &instance));
    }

    void createSurface() {
        VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
        sci.window = window;
        VK_CHECK(vkCreateAndroidSurfaceKHR(instance, &sci, nullptr, &surface));
    }

    void pickDevice() {
        uint32_t count = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, nullptr));
        std::vector<VkPhysicalDevice> devs(count);
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, devs.data()));

        for (auto d : devs) {
            // find graphics+present family
            uint32_t qcount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(d, &qcount, nullptr);
            std::vector<VkQueueFamilyProperties> qprops(qcount);
            vkGetPhysicalDeviceQueueFamilyProperties(d, &qcount, qprops.data());
            for (uint32_t i = 0; i < qcount; ++i) {
                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface, &present);
                if ((qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
                    phys = d;
                    gfxQueueFamily = i;
                    break;
                }
            }
            if (phys) break;
        }
        assert(phys != VK_NULL_HANDLE && "No suitable Vulkan device");
    }

    void createDevice() {
        float prio = 1.0f;
        VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        qci.queueFamilyIndex = gfxQueueFamily;
        qci.queueCount = 1;
        qci.pQueuePriorities = &prio;

        const char* exts[] = { "VK_KHR_swapchain" };

        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &qci;
        dci.enabledExtensionCount = 1;
        dci.ppEnabledExtensionNames = exts;

        VK_CHECK(vkCreateDevice(phys, &dci, nullptr, &device));
        vkGetDeviceQueue(device, gfxQueueFamily, 0, &gfxQueue);
    }

    void createSwapchain() {
        // Query surface format
        uint32_t fmtCount = 0;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &fmtCount, nullptr));
        std::vector<VkSurfaceFormatKHR> formats(fmtCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &fmtCount, formats.data()));

        VkSurfaceFormatKHR chosen = formats[0];
        for (auto& f : formats) {
            if ((f.format == VK_FORMAT_R8G8B8A8_UNORM || f.format == VK_FORMAT_B8G8R8A8_UNORM) &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosen = f;
                break;
            }
        }
        swapchainFormat = chosen.format;

        // Surface capabilities
        VkSurfaceCapabilitiesKHR caps{};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &caps));
        swapchainExtent = caps.currentExtent;
        if (swapchainExtent.width == UINT32_MAX) {
            // Fallback to window size
            swapchainExtent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, (uint32_t)ANativeWindow_getWidth(window)));
            swapchainExtent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, (uint32_t)ANativeWindow_getHeight(window)));
        }

        uint32_t desiredImages = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && desiredImages > caps.maxImageCount) desiredImages = caps.maxImageCount;

        // Present mode (FIFO guaranteed)
        uint32_t pmCount = 0;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &pmCount, nullptr));
        std::vector<VkPresentModeKHR> pms(pmCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &pmCount, pms.data()));
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

        VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        sci.surface = surface;
        sci.minImageCount = desiredImages;
        sci.imageFormat = swapchainFormat;
        sci.imageColorSpace = chosen.colorSpace;
        sci.imageExtent = swapchainExtent;
        sci.imageArrayLayers = 1;
        sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sci.preTransform = caps.currentTransform;
        sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        sci.presentMode = presentMode;
        sci.clipped = VK_TRUE;
        sci.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain));

        uint32_t imgCount = 0;
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr));
        swapchainImages.resize(imgCount);
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imgCount, swapchainImages.data()));

        swapchainViews.resize(imgCount);
        for (uint32_t i = 0; i < imgCount; ++i) {
            VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            ivci.image = swapchainImages[i];
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.format = swapchainFormat;
            ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ivci.subresourceRange.levelCount = 1;
            ivci.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &swapchainViews[i]));
        }
    }

    void createOnscreenRenderPass() {
        VkAttachmentDescription color{};
        color.format = swapchainFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;

        VkSubpassDependency dep{};
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.srcAccessMask = 0;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpci.attachmentCount = 1;
        rpci.pAttachments = &color;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &sub;
        rpci.dependencyCount = 1;
        rpci.pDependencies = &dep;

        VK_CHECK(vkCreateRenderPass(device, &rpci, nullptr, &onscreenRenderPass));
    }

    uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props) {
        VkPhysicalDeviceMemoryProperties mp{};
        vkGetPhysicalDeviceMemoryProperties(phys, &mp);
        for (uint32_t i = 0; i < mp.memoryTypeCount; ++i) {
            if ((typeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props)
                return i;
        }
        assert(false && "No suitable memory type");
        return 0;
    }

    void createOffscreenTarget() {
        // Image
        VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = offscreenFormat;
        ici.extent = {swapchainExtent.width, swapchainExtent.height, 1};
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(device, &ici, nullptr, &offscreenImage));

        VkMemoryRequirements mr{};
        vkGetImageMemoryRequirements(device, offscreenImage, &mr);

        VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        mai.allocationSize = mr.size;
        mai.memoryTypeIndex = findMemoryType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK(vkAllocateMemory(device, &mai, nullptr, &offscreenMemory));
        VK_CHECK(vkBindImageMemory(device, offscreenImage, offscreenMemory, 0));

        // Image view
        VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        ivci.image = offscreenImage;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = offscreenFormat;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &offscreenView));

        // Render pass
        VkAttachmentDescription color{};
        color.format = offscreenFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;

        VkSubpassDependency dep{};
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.srcAccessMask = 0;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpci.attachmentCount = 1;
        rpci.pAttachments = &color;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &sub;
        rpci.dependencyCount = 1;
        rpci.pDependencies = &dep;
        VK_CHECK(vkCreateRenderPass(device, &rpci, nullptr, &offscreenRenderPass));

        // Framebuffer
        VkImageView atts[] = { offscreenView };
        VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fbci.renderPass = offscreenRenderPass;
        fbci.attachmentCount = 1;
        fbci.pAttachments = atts;
        fbci.width = swapchainExtent.width;
        fbci.height = swapchainExtent.height;
        fbci.layers = 1;
        VK_CHECK(vkCreateFramebuffer(device, &fbci, nullptr, &offscreenFramebuffer));
    }

    void createDescriptorsAndSampler() {
        // Descriptor set layout: binding 0 = combined image sampler
        VkDescriptorSetLayoutBinding b{};
        b.binding = 0;
        b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        b.descriptorCount = 1;
        b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        lci.bindingCount = 1;
        lci.pBindings = &b;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &lci, nullptr, &descSetLayout));

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo dpci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        dpci.maxSets = 1;
        dpci.poolSizeCount = 1;
        dpci.pPoolSizes = &poolSize;
        VK_CHECK(vkCreateDescriptorPool(device, &dpci, nullptr, &descPool));

        VkDescriptorSetAllocateInfo dsai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        dsai.descriptorPool = descPool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = &descSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(device, &dsai, &descSet));

        VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sci.magFilter = VK_FILTER_LINEAR;
        sci.minFilter = VK_FILTER_LINEAR;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.minLod = 0.0f;
        sci.maxLod = 0.0f;
        VK_CHECK(vkCreateSampler(device, &sci, nullptr, &sampler));

        VkDescriptorImageInfo dii{};
        dii.sampler = sampler;
        dii.imageView = offscreenView;
        dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 我们在每帧采样前保证转换到这个布局

        VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w.dstSet = descSet;
        w.dstBinding = 0;
        w.dstArrayElement = 0;
        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        w.descriptorCount = 1;
        w.pImageInfo = &dii;

        vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
    }

    VkShaderModule makeShader(const uint32_t* code, size_t size) {
        VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        smci.codeSize = size;
        smci.pCode = code;
        VkShaderModule m = VK_NULL_HANDLE;
        VK_CHECK(vkCreateShaderModule(device, &smci, nullptr, &m));
        return m;
    }

    void createPipelines() {
        // Offscreen pipeline
        VkShaderModule vso = makeShader(SPV_offscreen_vert, SPV_offscreen_vert_size);
        VkShaderModule fso = makeShader(SPV_offscreen_frag, SPV_offscreen_frag_size);

        VkPipelineShaderStageCreateInfo stagesOff[2]{};
        stagesOff[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stagesOff[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stagesOff[0].module = vso;
        stagesOff[0].pName = "main";
        stagesOff[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stagesOff[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stagesOff[1].module = fso;
        stagesOff[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vp.viewportCount = 1;
        vp.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_NONE;
        rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rs.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState cba{};
        cba.colorWriteMask = 0xF;
        cba.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        cb.attachmentCount = 1;
        cb.pAttachments = &cba;

        VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dyn.dynamicStateCount = 2;
        dyn.pDynamicStates = dynStates;

        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VK_CHECK(vkCreatePipelineLayout(device, &plci, nullptr, &pipelineLayoutOffscreen));

        VkGraphicsPipelineCreateInfo pci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pci.stageCount = 2;
        pci.pStages = stagesOff;
        pci.pVertexInputState = &vi;
        pci.pInputAssemblyState = &ia;
        pci.pViewportState = &vp;
        pci.pRasterizationState = &rs;
        pci.pMultisampleState = &ms;
        pci.pColorBlendState = &cb;
        pci.pDynamicState = &dyn;
        pci.layout = pipelineLayoutOffscreen;
        pci.renderPass = offscreenRenderPass;
        pci.subpass = 0;

        VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pci, nullptr, &pipelineOffscreen));

        vkDestroyShaderModule(device, vso, nullptr);
        vkDestroyShaderModule(device, fso, nullptr);

        // Present pipeline (uses descriptor set 0 for texture)
        VkShaderModule vsp = makeShader(SPV_present_vert, SPV_present_vert_size);
        VkShaderModule fsp = makeShader(SPV_present_frag, SPV_present_frag_size);

        VkPipelineShaderStageCreateInfo stagesPr[2]{};
        stagesPr[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stagesPr[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stagesPr[0].module = vsp;
        stagesPr[0].pName = "main";
        stagesPr[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stagesPr[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stagesPr[1].module = fsp;
        stagesPr[1].pName = "main";

        VkPipelineLayoutCreateInfo plci2{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plci2.setLayoutCount = 1;
        plci2.pSetLayouts = &descSetLayout;
        VK_CHECK(vkCreatePipelineLayout(device, &plci2, nullptr, &pipelineLayoutPresent));

        pci = {};
        pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pci.stageCount = 2;
        pci.pStages = stagesPr;
        pci.pVertexInputState = &vi;
        pci.pInputAssemblyState = &ia;
        pci.pViewportState = &vp;
        pci.pRasterizationState = &rs;
        pci.pMultisampleState = &ms;
        pci.pColorBlendState = &cb;
        pci.pDynamicState = &dyn;
        pci.layout = pipelineLayoutPresent;
        pci.renderPass = onscreenRenderPass;
        pci.subpass = 0;

        VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pci, nullptr, &pipelinePresent));

        vkDestroyShaderModule(device, vsp, nullptr);
        vkDestroyShaderModule(device, fsp, nullptr);
    }

    void createFramebuffers() {
        swapchainFramebuffers.resize(swapchainViews.size());
        for (size_t i = 0; i < swapchainViews.size(); ++i) {
            VkImageView atts[] = { swapchainViews[i] };
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = onscreenRenderPass;
            fbci.attachmentCount = 1;
            fbci.pAttachments = atts;
            fbci.width = swapchainExtent.width;
            fbci.height = swapchainExtent.height;
            fbci.layers = 1;
            VK_CHECK(vkCreateFramebuffer(device, &fbci, nullptr, &swapchainFramebuffers[i]));
        }
    }

    void createCommandResources() {
        VkCommandPoolCreateInfo cpci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cpci.queueFamilyIndex = gfxQueueFamily;
        cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK(vkCreateCommandPool(device, &cpci, nullptr, &cmdPool));

        VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cbai.commandPool = cmdPool;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(device, &cbai, &cmd));

        VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK(vkCreateSemaphore(device, &sci, nullptr, &semImageAvailable));
        VK_CHECK(vkCreateSemaphore(device, &sci, nullptr, &semRenderFinished));

        VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(device, &fci, nullptr, &inFlight));
    }
};


// ---- Android entry ----
static void handle_cmd(android_app* app, int32_t cmd) {
    VulkanApp* vk = (VulkanApp*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window && !vk->running) {
                vk->init(app->window);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            if (vk->running) {
                vk->shutdown();
                vk->running = false;
            }
            break;
    }
}


void android_main(android_app* app) {
    app->onAppCmd = handle_cmd;
    VulkanApp vk{};
    app->userData = &vk;

    int events = 0;
    android_poll_source* source = nullptr;

    while (true) {
        // 如果正在运行，不阻塞（timeout=0）；否则阻塞等待事件（timeout=-1）
        int timeoutMillis = vk.running ? 0 : -1;

        int ident = ALooper_pollOnce(timeoutMillis, nullptr, &events, (void**)&source);
        // 处理本次得到的事件
        while (ident >= 0) {
            if (source) {
                source->process(app, source);
            }
            if (app->destroyRequested) {
                if (vk.running) vk.shutdown();
                return;
            }
            // 继续以非阻塞方式把队列中的剩余事件排空
            ident = ALooper_pollOnce(0, nullptr, &events, (void**)&source);
        }

        if (vk.running) {
            vk.drawFrame();
        }
    }
}