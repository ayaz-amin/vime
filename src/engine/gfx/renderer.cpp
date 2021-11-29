#include <vector>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include "renderer.h"

Renderer g_renderer = {};

// Utility functions

uint32_t clamp(uint32_t d, uint32_t min, uint32_t max) {
  const uint32_t t = d < min ? min : d;
  return t > max ? max : t;
}

#ifndef DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL v_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) 
{
    if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cout << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

void v_create_debug_messenger_info(VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_info)
{
    debug_messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_info.pNext = nullptr;
    debug_messenger_info.pUserData = nullptr;
    debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_messenger_info.pfnUserCallback = v_debug_callback;
}

VkResult v_init_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    v_create_debug_messenger_info(debug_messenger_info);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(g_renderer.m_instance, "vkCreateDebugUtilsMessengerEXT");
    
    if(func != nullptr) return func(g_renderer.m_instance,
        &debug_messenger_info, nullptr, &g_renderer.m_debug_messenger
    );
    else VK_ERROR_EXTENSION_NOT_PRESENT;
}

void v_destroy_debug_messenger()
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(g_renderer.m_instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if(func != nullptr) func(g_renderer.m_instance, g_renderer.m_debug_messenger, nullptr);
}
#endif

// Main API definitions

void v_init_instance(const char* app_name)
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pEngineName = "VIME";
    app_info.pApplicationName = app_name;

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    std::vector<char*> extension_names(extension_count);
    
    for(uint32_t i=0; i < extension_count; i++)
    {
        extension_names[i] = _strdup(extensions[i].extensionName);
    }

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

#ifndef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    v_create_debug_messenger_info(debug_messenger_info);
    instance_info.pNext = &debug_messenger_info;

    instance_info.enabledExtensionCount = extension_count + 1;
    extension_names.resize(extension_count + 1);
    extension_names[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    instance_info.ppEnabledExtensionNames = extension_names.data();

    const char* layer_names = "VK_LAYER_KHRONOS_validation";

    instance_info.enabledLayerCount = 1;
    instance_info.ppEnabledLayerNames = &layer_names;
#else
    instance_info.pNext = nullptr;
    instance_info.enabledLayerCount = 0;
    instance_info.ppEnabledLayerNames = nullptr;
#endif

    instance_info.enabledExtensionCount = extension_count;
    instance_info.ppEnabledExtensionNames = extension_names.data();

    vkCreateInstance(&instance_info, nullptr, &g_renderer.m_instance);

#ifndef DEBUG
    v_init_debug_messenger();
#endif 
}

void v_destroy_instance()
{
#ifndef DEBUG
    v_destroy_debug_messenger();
#endif
    vkDestroyInstance(g_renderer.m_instance, nullptr);
}

void v_init_surface(VkSurfaceKHR surface)
{
    g_renderer.m_surface_khr = surface;
}

void v_destroy_surface()
{
    vkDestroySurfaceKHR(g_renderer.m_instance, g_renderer.m_surface_khr, nullptr);
}

void v_init_device()
{
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(g_renderer.m_instance, &physical_device_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(g_renderer.m_instance, &physical_device_count, physical_devices.data());
    g_renderer.m_selected_device = physical_devices[0];

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_renderer.m_selected_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_renderer.m_selected_device, &queue_family_count, queue_families.data());

    VkBool32 graphics_support = VK_FALSE;
    VkBool32 present_support = VK_FALSE;
    for(uint32_t i=0; i < queue_family_count; i++)
    {
        if(!graphics_support)
        {
            if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                g_renderer.m_graphics_queue_family = i;
                graphics_support = VK_TRUE;
                continue;
            }
        }

        if(!present_support)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                g_renderer.m_selected_device, i, g_renderer.m_surface_khr, &present_support
            );
            if(present_support) g_renderer.m_present_queue_family = i; continue;
        }

        if(present_support && graphics_support)
        {
            break;
        }
    }

    float queue_priorities = 1.0f;

    VkDeviceQueueCreateInfo graphics_queue_info{};
    VkDeviceQueueCreateInfo present_queue_info{};
    
    graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_info.pNext = nullptr;
    graphics_queue_info.queueCount = 1;
    graphics_queue_info.queueFamilyIndex = g_renderer.m_graphics_queue_family;
    graphics_queue_info.pQueuePriorities = &queue_priorities;

    present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    present_queue_info.pNext = nullptr;
    present_queue_info.queueCount = 1;
    present_queue_info.queueFamilyIndex = g_renderer.m_present_queue_family;
    present_queue_info.pQueuePriorities = &queue_priorities;
    
    VkDeviceQueueCreateInfo queue_create_infos[] = {graphics_queue_info, present_queue_info};

    VkPhysicalDeviceFeatures device_features{};
    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = nullptr;
    device_info.queueCreateInfoCount = 2;
    device_info.pQueueCreateInfos = queue_create_infos;
    device_info.pEnabledFeatures = &device_features;

    const char* device_extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = &device_extension;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = nullptr;

    vkCreateDevice(g_renderer.m_selected_device, &device_info, nullptr, &g_renderer.m_device);
    vkGetDeviceQueue(g_renderer.m_device,
        g_renderer.m_graphics_queue_family,
        0,
        &g_renderer.m_graphics_queue
    );
    vkGetDeviceQueue(g_renderer.m_device,
        g_renderer.m_present_queue_family,
        0,
        &g_renderer.m_present_queue
    );
}

void v_destroy_device()
{
    vkDestroyDevice(g_renderer.m_device, nullptr);
}

void v_init_allocator()
{
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.physicalDevice = g_renderer.m_selected_device;
    allocator_info.device = g_renderer.m_device;
    allocator_info.instance = g_renderer.m_instance;
    vmaCreateAllocator(&allocator_info, &g_renderer.m_allocator);
}

void v_destroy_allocator()
{
    vmaDestroyAllocator(g_renderer.m_allocator);
}

void v_init_swapchain(uint32_t width, uint32_t height)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;

    VkBool32 support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        g_renderer.m_selected_device, g_renderer.m_graphics_queue_family,
        g_renderer.m_surface_khr, &support
    );

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        g_renderer.m_selected_device, g_renderer.m_surface_khr, &surface_capabilities
    );

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        g_renderer.m_selected_device, g_renderer.m_surface_khr, &format_count, nullptr
    );

    if(format_count != 0)
    {
        surface_formats.reserve(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            g_renderer.m_selected_device, g_renderer.m_surface_khr, &format_count, surface_formats.data()
        );
    }

    VkSurfaceFormatKHR surface_format;
    for(uint32_t i=0; i < format_count; i++)
    {
        surface_format = surface_formats[i];
        if(surface_format.format == VK_FORMAT_R8G8B8A8_SRGB 
        && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            break;
        } else surface_format = surface_formats[0];
    }

    if(surface_capabilities.currentExtent.width != UINT32_MAX) 
    {
        g_renderer.m_win_extent = surface_capabilities.currentExtent;
    } else {
        width = clamp(
            width,
            surface_capabilities.minImageExtent.width,
            surface_capabilities.maxImageExtent.width
        ); height = clamp(
            height,
            surface_capabilities.minImageExtent.height,
            surface_capabilities.maxImageExtent.height
        );
        g_renderer.m_win_extent.width = width;
        g_renderer.m_win_extent.height = height;
    }

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if(surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
    {
        image_count = surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info{};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext = nullptr;
    swapchain_info.surface = g_renderer.m_surface_khr;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = surface_format.format;
    swapchain_info.imageColorSpace = surface_format.colorSpace;
    swapchain_info.imageExtent = g_renderer.m_win_extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if(g_renderer.m_graphics_queue_family != g_renderer.m_present_queue_family)
    {
        uint32_t queue_family_indices[] = {
            g_renderer.m_graphics_queue_family,
            g_renderer.m_present_queue_family
        };

        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_info.preTransform = surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(
        g_renderer.m_device, &swapchain_info, nullptr, &g_renderer.m_swapchain
    );

    vkGetSwapchainImagesKHR(
        g_renderer.m_device, g_renderer.m_swapchain, &image_count, nullptr
    ); g_renderer.m_swapchain_images.reserve(image_count);
    
    vkGetSwapchainImagesKHR(
        g_renderer.m_device, g_renderer.m_swapchain, &image_count, g_renderer.m_swapchain_images.data()
    );

    g_renderer.m_swapchain_image_size = image_count;
    g_renderer.m_swapchain_image_format = surface_format.format;
}

void v_destroy_swapchain()
{
    vkDestroySwapchainKHR(
        g_renderer.m_device, g_renderer.m_swapchain, nullptr
    );
}

void v_init_render_pass()
{
    VkAttachmentDescription attachment{};
    attachment.format = g_renderer.m_swapchain_image_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_ref{};
    attachment_ref.attachment = 0;
    attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_ref;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    vkCreateRenderPass(g_renderer.m_device, &render_pass_info, nullptr, &g_renderer.m_render_pass);
}

void v_destroy_render_pass()
{
    vkDestroyRenderPass(g_renderer.m_device, g_renderer.m_render_pass, nullptr);
}

void v_init_cmd_pool()
{
    VkCommandPoolCreateInfo cmd_pool_info{};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = nullptr;
    cmd_pool_info.queueFamilyIndex = g_renderer.m_graphics_queue_family;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(g_renderer.m_device, &cmd_pool_info, nullptr, &g_renderer.m_command_pool);
}

void v_destroy_cmd_pool()
{
    vkDestroyCommandPool(g_renderer.m_device, g_renderer.m_command_pool, nullptr);
}

void v_allocate_cmd_buffer()
{
    VkCommandBufferAllocateInfo cmd_buffer_info{};
    cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_info.pNext = nullptr;
    cmd_buffer_info.commandBufferCount = 1;
    cmd_buffer_info.commandPool = g_renderer.m_command_pool;
    cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(g_renderer.m_device, &cmd_buffer_info, &g_renderer.m_command_buffer);
}

void v_init_framebuffers()
{
    g_renderer.m_swapchain_image_views.reserve(g_renderer.m_swapchain_image_size);
    g_renderer.m_framebuffers.reserve(g_renderer.m_swapchain_image_size);
    
    for(uint32_t i=0; i < g_renderer.m_swapchain_image_size; i++)
    {
        VkImageViewCreateInfo iv_info{};
        iv_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv_info.pNext = nullptr;
        iv_info.image = g_renderer.m_swapchain_images[i];
        iv_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv_info.format = g_renderer.m_swapchain_image_format;
        iv_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv_info.subresourceRange.baseMipLevel = 0;
        iv_info.subresourceRange.levelCount = 1;
        iv_info.subresourceRange.baseArrayLayer = 0;
        iv_info.subresourceRange.layerCount = 1;

        vkCreateImageView(
            g_renderer.m_device, &iv_info, nullptr, &g_renderer.m_swapchain_image_views[i]
        );

        VkFramebufferCreateInfo fb_info{};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;
        fb_info.renderPass = g_renderer.m_render_pass;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = &g_renderer.m_swapchain_image_views[i];
        fb_info.width = g_renderer.m_win_extent.width;
        fb_info.height = g_renderer.m_win_extent.height;
        fb_info.layers = 1;

        vkCreateFramebuffer(
            g_renderer.m_device, &fb_info, nullptr, &g_renderer.m_framebuffers[i]
        );
    }
}

void v_destroy_framebuffers()
{
    for(uint32_t i=0; i < g_renderer.m_swapchain_image_size; i++)
    {
        vkDestroyFramebuffer(
            g_renderer.m_device, g_renderer.m_framebuffers[i], nullptr
        );

        vkDestroyImageView(
            g_renderer.m_device, g_renderer.m_swapchain_image_views[i], nullptr
        );
    }
}

void v_init_sync_structs()
{
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    vkCreateFence(g_renderer.m_device, &fence_info, nullptr, &g_renderer.m_render_fence);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = nullptr;
    semaphore_info.flags = 0;

    vkCreateSemaphore(g_renderer.m_device, &semaphore_info, nullptr, &g_renderer.m_render_semaphore);
    vkCreateSemaphore(g_renderer.m_device, &semaphore_info, nullptr, &g_renderer.m_present_semaphore);
}

void v_destroy_sync_structs()
{
    vkDestroySemaphore(g_renderer.m_device, g_renderer.m_present_semaphore, nullptr);
    vkDestroySemaphore(g_renderer.m_device, g_renderer.m_render_semaphore, nullptr);
    vkDestroyFence(g_renderer.m_device, g_renderer.m_render_fence, nullptr);
}

void v_begin_rendering(ClearValue clear_value)
{
    vkWaitForFences(g_renderer.m_device, 1, &g_renderer.m_render_fence, true, 1000000000);
    vkResetFences(g_renderer.m_device, 1, &g_renderer.m_render_fence);

    vkAcquireNextImageKHR(
        g_renderer.m_device, g_renderer.m_swapchain, 1000000000,
        g_renderer.m_present_semaphore, nullptr, &g_renderer.m_swapchain_image_idx
    );

    vkResetCommandBuffer(g_renderer.m_command_buffer, 0);

    VkCommandBufferBeginInfo cmd_begin_info{};
    cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_begin_info.pNext = nullptr;
    cmd_begin_info.pInheritanceInfo = nullptr;
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(g_renderer.m_command_buffer, &cmd_begin_info);
    
    VkClearValue vk_clear_value;
    vk_clear_value.color = {{clear_value.R, clear_value.G, clear_value.B, clear_value.A}};
    VkRenderPassBeginInfo renderpass_begin_info{};
    renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.pNext = nullptr;
    renderpass_begin_info.renderPass = g_renderer.m_render_pass;
    renderpass_begin_info.renderArea.offset.x = 0;
    renderpass_begin_info.renderArea.offset.y = 0;
    renderpass_begin_info.renderArea.extent = g_renderer.m_win_extent;
    renderpass_begin_info.framebuffer = g_renderer.m_framebuffers[g_renderer.m_swapchain_image_idx];
    renderpass_begin_info.clearValueCount = 1;
    renderpass_begin_info.pClearValues = &vk_clear_value;

    vkCmdBeginRenderPass(
        g_renderer.m_command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE
    );
}

void v_end_rendering()
{
    vkCmdEndRenderPass(g_renderer.m_command_buffer);
    vkEndCommandBuffer(g_renderer.m_command_buffer);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &g_renderer.m_present_semaphore;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &g_renderer.m_render_semaphore;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &g_renderer.m_command_buffer;
    
    vkQueueSubmit(
        g_renderer.m_graphics_queue, 1, &submit_info, g_renderer.m_render_fence
    );

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &g_renderer.m_swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &g_renderer.m_render_semaphore;
    present_info.pImageIndices = &g_renderer.m_swapchain_image_idx;

    vkQueuePresentKHR(
        g_renderer.m_graphics_queue, &present_info
    );
}

void v_wait_for_fences()
{
    vkWaitForFences(g_renderer.m_device, 1, &g_renderer.m_render_fence, true, 1000000000);
}