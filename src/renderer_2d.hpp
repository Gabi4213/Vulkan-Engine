// renderer2d.hpp
#pragma once
#include "device.hpp"
#include "pipeline.hpp"
#include "buffers.hpp"
#include "ecs_registry.hpp"

#include <memory>

namespace lavander
{
    class Renderer2D
    {
    public:
        Renderer2D(c_device& device, VkRenderPass renderPass, VkExtent2D extent, VkPipelineLayout layout);
        void draw(VkCommandBuffer cmd, ECSRegistry& registry);

    private:
        c_device& deviceRef; // store device
        VkPipelineLayout        pipelineLayout;
        std::unique_ptr<c_pipeline> pipeline;
        std::unique_ptr<c_buffers> quadBuffers;

        void createPipeline(VkRenderPass renderPass, VkExtent2D extent);
        void createQuadBuffers();
    };
}
