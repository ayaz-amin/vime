#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

// Main API

typedef struct
{
    float R;
    float G;
    float B;
    float A;
} ClearValue;

struct Renderer
{
    VkExtent2D m_win_extent;
    VkInstance m_instance;
    #ifndef DEBUG
    VkDebugUtilsMessengerEXT m_debug_messenger;
    #endif
    VkSurfaceKHR m_surface_khr;
    VkPhysicalDevice m_selected_device;    
    VkDevice m_device;
        
    uint32_t m_graphics_queue_family;
    uint32_t m_present_queue_family;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;

    VkSwapchainKHR m_swapchain;
    uint32_t m_swapchain_image_size;
    VkFormat m_swapchain_image_format;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::vector<VkFramebuffer> m_framebuffers;

    VkRenderPass m_render_pass;
    VkCommandPool m_command_pool;
    VkCommandBuffer m_command_buffer;

    uint32_t m_swapchain_image_idx;
    VkSemaphore m_render_semaphore;
    VkSemaphore m_present_semaphore;
    VkFence m_render_fence;

    VmaAllocator m_allocator;
};

extern Renderer g_renderer;

void v_init_instance(const char* app_name);
void v_destroy_instance();

#ifndef DEBUG
VkResult v_init_debug_messenger();
void v_destroy_debug_messenger();
#endif

void v_init_surface(VkSurfaceKHR surface);
void v_destroy_surface();

void v_init_device();
void v_destroy_device();

void v_init_allocator();
void v_destroy_allocator();

void v_init_swapchain(uint32_t width, uint32_t height);
void v_destroy_swapchain();

void v_init_render_pass();
void v_destroy_render_pass();

void v_init_cmd_pool();
void v_destroy_cmd_pool();

void v_allocate_cmd_buffer();

void v_init_framebuffers();
void v_destroy_framebuffers();

void v_init_sync_structs();
void v_destroy_sync_structs();

// Drawing code

void v_begin_rendering(ClearValue clear_value);
void v_end_rendering();
void v_wait_for_fences();