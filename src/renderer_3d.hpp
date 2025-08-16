#pragma once
#include "pipeline.hpp"
#include "device.hpp"
#include "texture2d.hpp"
#include "ecs_registry.hpp"
#include "mesh.hpp"

namespace lavander
{
    class Renderer3D 
    {
    public:
        Renderer3D(c_device& device, VkRenderPass renderPass, VkExtent2D extent, VkPipelineLayout pipelineLayout, VkDescriptorSetLayout materialSetLayout, VkDescriptorPool materialPool);
        
        void draw(VkCommandBuffer cmd, ECSRegistry& registry);

    private:
        void createPipeline(VkRenderPass rp, VkExtent2D extent);

        c_device& deviceRef;
        std::unique_ptr<c_pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout materialSetLayout;
        VkDescriptorPool materialPool;
        std::shared_ptr<Texture2D> defaultWhite;
    };
}