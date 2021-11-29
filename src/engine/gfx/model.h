#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <HandmadeMath.h>

#include "buffer.h"

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> m_bindings;
    std::vector<VkVertexInputAttributeDescription> m_attributes;
    VkPipelineVertexInputStateCreateFlags m_flags = 0;
};

struct Vertex
{
    hmm_vec3 m_position;
    hmm_vec3 m_color;
    hmm_vec3 m_normal;
};

struct Model
{
    std::vector<Vertex> m_vertices;
    AllocatedBuffer m_vertex_buffer;
};

VertexInputDescription v_get_vertex_decription();
Model v_load_model(const char* file_path);
void v_destroy_model(Model model);