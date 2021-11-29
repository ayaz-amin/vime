#pragma once

#include <vulkan/vulkan.h>

struct GraphicsPipeline
{
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_pipeline;
};

GraphicsPipeline v_create_graphics_pipeline(const char* vertex_path, const char* fragment_path);
void v_destroy_graphics_pipeline(GraphicsPipeline pipeline);