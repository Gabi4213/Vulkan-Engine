#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "buffers.hpp"
#include "ecs_registry.hpp"

#include <memory>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


namespace lavander {

    class Engine 
    {
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine&operator=(const Engine&) = delete;
        
        void run();

        ECSRegistry& getRegistry() { return registry; }

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

    };
}