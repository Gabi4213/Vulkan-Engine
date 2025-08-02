#include "buffers.hpp"
#include <cstring>

namespace lavander 
{

    c_buffers::c_buffers(c_device& device,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices)
        : deviceRef(device),
        vertexCount(static_cast<uint32_t>(vertices.size())),
        indexCount(static_cast<uint32_t>(indices.size())),
        hasIndexBuffer(!indices.empty()) 
    {

        // Vertex Buffer
        VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

        deviceRef.createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory);

        void* vertexData;
        vkMapMemory(deviceRef.device(), vertexBufferMemory, 0, vertexBufferSize, 0, &vertexData);
        memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBufferSize));
        vkUnmapMemory(deviceRef.device(), vertexBufferMemory);


        if (hasIndexBuffer) 
        {
            VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

            deviceRef.createBuffer(
                indexBufferSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                indexBuffer,
                indexBufferMemory);

            void* indexData;
            vkMapMemory(deviceRef.device(), indexBufferMemory, 0, indexBufferSize, 0, &indexData);
            memcpy(indexData, indices.data(), static_cast<size_t>(indexBufferSize));
            vkUnmapMemory(deviceRef.device(), indexBufferMemory);
        }
    }

    c_buffers::~c_buffers()
    {
        if (hasIndexBuffer)
        {
            vkDestroyBuffer(deviceRef.device(), indexBuffer, nullptr);
            vkFreeMemory(deviceRef.device(), indexBufferMemory, nullptr);
        }

        vkDestroyBuffer(deviceRef.device(), vertexBuffer, nullptr);
        vkFreeMemory(deviceRef.device(), vertexBufferMemory, nullptr);
    }

    void c_buffers::bind(VkCommandBuffer commandBuffer) 
    {
        VkBuffer buffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void c_buffers::draw(VkCommandBuffer commandBuffer)
    {
        if (hasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

}
