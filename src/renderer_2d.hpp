// renderer2d.hpp
#pragma once
#include "device.hpp"
#include "pipeline.hpp"
#include "buffers.hpp"
#include "ecs_registry.hpp"
#include "texture2d.hpp"
#include <memory>

namespace lavander
{
    class Renderer2D
    {
    public:
        Renderer2D(c_device& device, VkRenderPass renderPass, VkExtent2D extent, VkPipelineLayout layout, VkDescriptorSetLayout materialSetLayout, VkDescriptorPool materialPool);
        void draw(VkCommandBuffer cmd, ECSRegistry& registry);

    private:
        c_device& deviceRef;
        VkPipelineLayout        pipelineLayout;
        std::unique_ptr<c_pipeline> pipeline;
        std::unique_ptr<c_buffers> quadBuffers;

        VkDescriptorSetLayout materialSetLayout{};
        VkDescriptorPool materialPool{};
        std::shared_ptr<Texture2D>  defaultWhite;
        VkDescriptorSet             defaultWhiteSet{};

        void createPipeline(VkRenderPass renderPass, VkExtent2D extent);
        void createQuadBuffers();
        void createDefaultTexture();
    };
}
