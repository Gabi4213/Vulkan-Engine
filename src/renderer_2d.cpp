#include "renderer_2d.hpp"
#include "vertex.hpp"
#include "device.hpp"
#include "component_storage.hpp"
#include "components.hpp"
#include "ecs_registry.hpp"

#include <stdexcept>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace lavander 
{

    Renderer2D::Renderer2D(c_device& device,
        VkRenderPass renderPass,
        VkExtent2D extent,
        VkPipelineLayout layout,
        VkDescriptorSetLayout matLayout,
        VkDescriptorPool matPool)
        : deviceRef(device),
        pipelineLayout(layout),
        materialSetLayout(matLayout),
        materialPool(matPool)        // comes from Engine
    {
        createQuadBuffers();
        createPipeline(renderPass, extent);
        createDefaultTexture();        // uses materialPool + materialSetLayout
    }

    void Renderer2D::createQuadBuffers()
    {
        std::vector<Vertex> vertices = 
        {
            {{-0.5f,-0.5f},{1,0,0},{0,0}}, //bottom-left
            {{ 0.5f,-0.5f},{0,1,0},{1,0}}, //bottom-right
            {{ 0.5f, 0.5f},{0,0,1},{1,1}}, //top-right
            {{-0.5f, 0.5f},{1,1,1},{0,1}}, //top-left
        };

        std::vector<uint32_t> indices = {
            0, 1, 2,
            2, 3, 0
        };

        quadBuffers = std::make_unique<c_buffers>(deviceRef, vertices, indices);
    }

    void Renderer2D::createDefaultTexture()
    {
        defaultWhite = std::make_shared<Texture2D>(deviceRef, 255, 255, 255, 255);
        // allocate once, cache inside Texture2D
        defaultWhite->allocateDescriptor(materialPool, materialSetLayout);
    }

    void Renderer2D::createPipeline(VkRenderPass renderPass, VkExtent2D extent)
    {
        auto config = c_pipeline::defaultPipelineConfigInfo(extent.width, extent.height);
        config.renderPass = renderPass;
        config.pipelineLayout = pipelineLayout;
        config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;

        pipeline = std::make_unique<c_pipeline>(
            deviceRef,
            "../../src/shaders/quad_shader.vert.spv",
            "../../src/shaders/quad_shader.frag.spv",
            config
        );
    }

    void Renderer2D::draw(VkCommandBuffer cmd, ECSRegistry& registry)
    {
        pipeline->bind(cmd);
        quadBuffers->bind(cmd);

        auto& spritesByEntity = registry.getAllComponentsOfType<SpriteRenderer>();
        for (auto& [entity, spriteList] : spritesByEntity)
        {
            auto* transforms = registry.getComponents<Transform>(entity);
            if (!transforms) continue;

            for (auto& sprite : spriteList)
            {
                // Choose descriptor set: default white, or sprite's texture
                VkDescriptorSet matSet = defaultWhite->descriptorSet();
                if (sprite.texture)
                {
                    // Allocate once if needed, then reuse cached set
                    if (sprite.texture->descriptorSet() == VK_NULL_HANDLE)
                        sprite.texture->allocateDescriptor(materialPool, materialSetLayout);

                    matSet = sprite.texture->descriptorSet();
                }

                // Bind material set at set = 1
                vkCmdBindDescriptorSets(
                    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, /*firstSet*/ 1, 1, &matSet,
                    0, nullptr
                );

                // Draw for each transform on this entity (supports multi-Transform)
                for (auto& t : *transforms)
                {
                    glm::mat4 model(1.0f);
                    model = glm::translate(model, t.position);
                    model = model
                        * glm::rotate(glm::mat4(1.0f), t.rotation.z, glm::vec3(0, 0, 1))
                        * glm::rotate(glm::mat4(1.0f), t.rotation.y, glm::vec3(0, 1, 0))
                        * glm::rotate(glm::mat4(1.0f), t.rotation.x, glm::vec3(1, 0, 0));
                    model = glm::scale(model, t.scale);

                    PushConst pc{};
                    pc.model = model;
                    pc.color = glm::vec4(sprite.color, 1.0f);

                    vkCmdPushConstants(
                        cmd, pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0, sizeof(PushConst), &pc
                    );

                    quadBuffers->draw(cmd);
                }
            }
        }
    }
}