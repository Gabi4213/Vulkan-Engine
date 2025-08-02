#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "vertex_buffer.hpp"

#include <memory>
#include <vector>

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

        private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void initVertexBuffer();
        void drawFrame();

        c_window window{WIDTH, HEIGHT, "Engine"};
        c_device device{window};
        c_swapchain swapChain{device, window.getExtent()};
        std::unique_ptr<c_pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;

        std::unique_ptr<c_vertex_buffer> vertexBuffer;

    };
}