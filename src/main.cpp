#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <GLFW/glfw3.h>

#include "engine/gfx/renderer.h"
#include "engine/gfx/model.h"
#include "engine/gfx/pipeline.h"
#include "engine/gfx/push_constant.h"

GLFWwindow* g_window;
VkSurfaceKHR g_surface;
GraphicsPipeline g_pipeline;

void draw(Model& model, GraphicsPipeline& pipeline, PushConstant constants)
{
    vkCmdBindPipeline(g_renderer.m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_pipeline);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(g_renderer.m_command_buffer, 0, 1, &model.m_vertex_buffer.m_buffer, &offset);
    vkCmdPushConstants(g_renderer.m_command_buffer, pipeline.m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &constants);
    vkCmdDraw(g_renderer.m_command_buffer, (uint32_t)model.m_vertices.size(), 1, 0, 0);
}

int main()
{
    int width = 800;
    int height = 600;
    const char* app_name = "Engine";

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    g_window = glfwCreateWindow(width, height, app_name, NULL, NULL);

    v_init_instance(app_name);
    
    glfwCreateWindowSurface(g_renderer.m_instance, g_window, NULL, &g_surface);
    
    v_init_surface(g_surface);
    v_init_device();
    v_init_allocator();
    v_init_swapchain((uint32_t)width, (uint32_t)height);
    v_init_render_pass();
    v_init_cmd_pool();
    v_allocate_cmd_buffer();
    v_init_framebuffers();
    v_init_sync_structs();
    
    g_pipeline = v_create_graphics_pipeline(
        "shaders/vertex.spv",
        "shaders/frag.spv"
    );

    Model mesh = v_load_model("assets/model.obj");
    PushConstant constants;
    float rotation = 0.0f;

    while(!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();
        if(glfwGetKey(g_window, GLFW_KEY_ESCAPE)) break;

        if(rotation >= 360.0f) rotation = 0.0f;
        rotation += 2.0f;

        hmm_vec3 cam_pos = {0.0f, 0.0f, -2.0f};
        hmm_mat4 view = HMM_Translate(cam_pos);
        hmm_mat4 projection = HMM_Perspective(70.0f, float(width / height), 0.1f, 200.0f);
        hmm_mat4 model = HMM_Rotate(rotation, HMM_Vec3(0, 1, 0));
        hmm_mat4 mesh_matrix = projection * view * model;
        constants.m_render_matrix = mesh_matrix;

        v_begin_rendering({0.4f, 0.5f, 0.6f, 1.0f});

        draw(mesh, g_pipeline, constants);

        v_end_rendering();
    }

    v_wait_for_fences();
    v_destroy_model(mesh);
    v_destroy_graphics_pipeline(g_pipeline);

    v_destroy_sync_structs();
    v_destroy_framebuffers();
    v_destroy_cmd_pool();
    v_destroy_render_pass();
    v_destroy_swapchain();
    v_destroy_allocator();
    v_destroy_device();
    v_destroy_surface();
    v_destroy_instance();

    glfwDestroyWindow(g_window);
    glfwTerminate();

    return 0;
}