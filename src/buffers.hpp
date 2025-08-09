#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "device.hpp"

#include <array>


namespace lavander 
{
    struct Vertex 
    {
        glm::vec2 position;
        glm::vec3 color;
        glm::vec2 uv;

        static VkVertexInputBindingDescription getBindingDescription() 
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0; attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };


    class c_buffers 
    {
    public:
        c_buffers(c_device& device,
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices = {});
        ~c_buffers();

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        c_device& deviceRef;

        VkBuffer vertexBuffer{};
        VkDeviceMemory vertexBufferMemory{};
        uint32_t vertexCount{};

        VkBuffer indexBuffer{};
        VkDeviceMemory indexBufferMemory{};
        uint32_t indexCount{};
        bool hasIndexBuffer = false;
    };


}