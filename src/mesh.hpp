#pragma once
#include "buffers.hpp"
#include <memory>

namespace lavander
{
    class Mesh
    {
    public:

        Mesh(c_device& dev, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
        void bind(VkCommandBuffer cmd) { buffers->bind(cmd); }
        void draw(VkCommandBuffer cmd) { buffers->draw(cmd); }

        static std::shared_ptr<Mesh> MakeCube(c_device& dev);

    private:
        std::unique_ptr<c_buffers> buffers;
    };
}
