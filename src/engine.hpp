#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "buffers.hpp"
#include "ecs_registry.hpp"
#include "renderer_2d.hpp"
#include "scene_graph.hpp"

#include <memory>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>



namespace lavander {

    class Engine 
    {
        public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;
        
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine&operator=(const Engine&) = delete;
        
        void run();

        ECSRegistry& getRegistry() { return registry; }

        SceneGraph sceneGraph{ &registry };

        private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void initBuffers();
        void drawFrame();
        void createDescriptorSetLayout();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout);
        void updateUniformBuffer(uint32_t currentImage);

        void allocateCommandBuffers();
        void recordCommandBuffer(int imageIndex);

        c_window window{WIDTH, HEIGHT, "Engine"};
        c_device device{window};
        c_swapchain swapChain{device, window.getExtent()};
        std::unique_ptr<c_pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;

        std::unique_ptr<c_buffers> buffers;


        VkDescriptorSetLayout descriptorSetLayout;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;

        ECSRegistry registry;
        std::unique_ptr<Renderer2D> renderer2D;
        

        VkDescriptorPool imguiPool = VK_NULL_HANDLE;

        void createImGuiDescriptorPool();
        void initImGui();
        void shutdownImGui();
        void uploadImGuiFonts();

    };
}