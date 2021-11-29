#include <string>
#include <vk_mem_alloc.h>
#include <tiny_obj_loader.h>

#include "model.h"
#include "renderer.h"

VertexInputDescription v_get_vertex_decription()
{
    VertexInputDescription description;

    VkVertexInputBindingDescription vertex_binding{};
    vertex_binding.binding = 0;
    vertex_binding.stride = sizeof(Vertex);
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription position_attribute{};
    position_attribute.binding = 0;
    position_attribute.location = 0;
    position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    position_attribute.offset = offsetof(Vertex, m_position);

    VkVertexInputAttributeDescription color_attribute{};
    color_attribute.binding = 0;
    color_attribute.location = 1;
    color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attribute.offset = offsetof(Vertex, m_color);

    VkVertexInputAttributeDescription normal_attribute{};
    normal_attribute.binding = 0;
    normal_attribute.location = 2;
    normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attribute.offset = offsetof(Vertex, m_normal);

    description.m_bindings.push_back(vertex_binding);
    description.m_attributes.push_back(position_attribute);
    description.m_attributes.push_back(color_attribute);
    description.m_attributes.push_back(normal_attribute);

    return description;
}

Model v_load_model(const char* file_path)
{
    Model model;

    tinyobj::attrib_t vertex_attribute;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    tinyobj::LoadObj(&vertex_attribute, &shapes, &materials, &warning, &error, file_path, nullptr);

    for(const auto& shape : shapes)
    {
        for(const auto& idx: shape.mesh.indices)
        {
            Vertex new_vertex;
                
            new_vertex.m_position.X = vertex_attribute.vertices[3 * idx.vertex_index + 0];
            new_vertex.m_position.Y = vertex_attribute.vertices[3 * idx.vertex_index + 1];
            new_vertex.m_position.Z = vertex_attribute.vertices[3 * idx.vertex_index + 2];

            new_vertex.m_normal.X = vertex_attribute.normals[3 * idx.vertex_index + 0];
            new_vertex.m_normal.Y = vertex_attribute.normals[3 * idx.vertex_index + 1];
            new_vertex.m_normal.Z = vertex_attribute.normals[3 * idx.vertex_index + 2];

            new_vertex.m_color = new_vertex.m_normal;

            model.m_vertices.push_back(new_vertex);

        }
    }

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = nullptr;
    buffer_info.size = model.m_vertices.size() * sizeof(Vertex);
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(g_renderer.m_allocator, &buffer_info, &allocation_info,
        &model.m_vertex_buffer.m_buffer, &model.m_vertex_buffer.m_allocation, nullptr);

    void* data;
    vmaMapMemory(g_renderer.m_allocator, model.m_vertex_buffer.m_allocation, &data);
    memcpy(data, model.m_vertices.data(), model.m_vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(g_renderer.m_allocator, model.m_vertex_buffer.m_allocation);

    return model;
}

void v_destroy_model(Model model)
{
    vmaDestroyBuffer(g_renderer.m_allocator, model.m_vertex_buffer.m_buffer, model.m_vertex_buffer.m_allocation);
}