#include "vertex_buffer.hpp"
#include <cstring>

namespace lavander
{

    c_vertex_buffer::c_vertex_buffer(c_device& device, const std::vector<Vertex>& vertices): deviceRef(device), vertexCount(static_cast<uint32_t>(vertices.size())) 
    {

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        deviceRef.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );

        void* data;
        vkMapMemory(deviceRef.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(deviceRef.device(), vertexBufferMemory);
    }

    c_vertex_buffer::~c_vertex_buffer() 
    {
        vkDestroyBuffer(deviceRef.device(), vertexBuffer, nullptr);
        vkFreeMemory(deviceRef.device(), vertexBufferMemory, nullptr);
    }

    void c_vertex_buffer::bind(VkCommandBuffer commandBuffer) 
    {
        VkBuffer buffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void c_vertex_buffer::draw(VkCommandBuffer commandBuffer) 
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

}
