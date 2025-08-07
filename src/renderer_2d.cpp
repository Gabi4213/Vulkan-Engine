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

    Renderer2D::Renderer2D(c_device& device, VkRenderPass renderPass, VkExtent2D extent, VkPipelineLayout layout): deviceRef(device), pipelineLayout(layout)
    {
        createQuadBuffers();
        createPipeline(renderPass, extent);
    }

    void Renderer2D::createQuadBuffers()
    {
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // bottom left
            {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // bottom right
            {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // top right
            {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}, // top left
        };

        std::vector<uint32_t> indices = {
            0, 1, 2,
            2, 3, 0
        };

        quadBuffers = std::make_unique<c_buffers>(deviceRef, vertices, indices);
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

        auto& sprites = registry.getAllComponentsOfType<SpriteRenderer>();
        for (auto& [entity, spriteList] : sprites)
        {
            auto* transformList = registry.getComponents<Transform>(entity);
            if (!transformList) continue;

            for (auto& t : *transformList)
            {
                for (auto& sprite : spriteList)
                {
                    glm::mat4 model = glm::mat4(1.0f);

                    //trasnsform
                    model = glm::translate(model, t.position);

                    //rotation
                    model = glm::rotate(model, t.rotation.z, glm::vec3(0, 0, 1));
                    model = glm::rotate(model, t.rotation.y, glm::vec3(0, 1, 0));
                    model = glm::rotate(model, t.rotation.x, glm::vec3(1, 0, 0));

                    //scale
                    model = glm::scale(model, t.scale);

                    PushConst pc{ model, glm::vec4(sprite.color, 1.0f) };

                    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConst), &pc);

                    quadBuffers->draw(cmd);
                }
            }
        }
    }
}