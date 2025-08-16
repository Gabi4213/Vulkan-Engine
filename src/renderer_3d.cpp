// renderer_3d.cpp
#include "renderer_3d.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "components.hpp"

namespace lavander
{
    Renderer3D::Renderer3D(c_device& device, VkRenderPass renderPass, VkExtent2D extent, VkPipelineLayout layout, VkDescriptorSetLayout matLayout, VkDescriptorPool matPool)
        : deviceRef(device), pipelineLayout(layout), materialSetLayout(matLayout), materialPool(matPool)
    {
        defaultWhite = std::make_shared<Texture2D>(deviceRef, 255, 255, 255, 255);
        defaultWhite->allocateDescriptor(materialPool, materialSetLayout);
        createPipeline(renderPass, extent);
    }

    void Renderer3D::createPipeline(VkRenderPass rp, VkExtent2D extent)
    {
        auto cfg = c_pipeline::defaultPipelineConfigInfo(extent.width, extent.height);
        cfg.renderPass = rp;
        cfg.pipelineLayout = pipelineLayout;

        //backface culling
        cfg.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        cfg.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

        cfg.colorBlendInfo.pAttachments = &cfg.colorBlendAttachment;

        //vertex layout
        auto bind = Vertex3D::getBindingDescription();
        auto attrs = Vertex3D::getAttributeDescriptions();
        cfg.bindingDescriptions = { bind };
        cfg.attributeDescriptions = { attrs.begin(), attrs.end() };

        pipeline = std::make_unique<c_pipeline>(
            deviceRef,
            "../../src/shaders/mesh.vert.spv",
            "../../src/shaders/mesh.frag.spv",
            cfg
        );
    }

    void Renderer3D::draw(VkCommandBuffer cmd, ECSRegistry& registry)
    {
        pipeline->bind(cmd);

        auto& meshrByEntity = registry.getAllComponentsOfType<MeshRenderer3D>();
        for (auto& [entity, renderers] : meshrByEntity)
        {
            auto* filters = registry.getComponents<MeshFilter>(entity);
            auto* transfs = registry.getComponents<Transform>(entity);

            if (!filters || !transfs) continue;

            for (auto& r : renderers)
            {
                VkDescriptorSet matSet = defaultWhite->descriptorSet();
                if (r.texture)
                {
                    if (r.texture->descriptorSet() == VK_NULL_HANDLE) 
                    {
                        r.texture->allocateDescriptor(materialPool, materialSetLayout);
                    }
                    matSet = r.texture->descriptorSet();
                }

                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &matSet, 0, nullptr);

                for (auto& t : *transfs)
                {
                    glm::mat4 model(1.0f);
                    model = glm::translate(model, t.position)
                        * glm::rotate(glm::mat4(1.0f), t.rotation.z, { 0,0,1 })
                        * glm::rotate(glm::mat4(1.0f), t.rotation.y, { 0,1,0 })
                        * glm::rotate(glm::mat4(1.0f), t.rotation.x, { 1,0,0 });
                    model = glm::scale(model, t.scale);

                    PushConst pc{};
                    pc.model = model;
                    pc.color = glm::vec4(r.color, 1.0f);

                    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConst), &pc);

                    for (auto& f : *filters)
                    {
                        if (!f.mesh) continue;
                        f.mesh->bind(cmd);
                        f.mesh->draw(cmd);
                    }
                }
            }
        }
    }
}