#include "mesh.hpp"

namespace lavander
{

    Mesh::Mesh(c_device& dev, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) : buffers(std::make_unique<c_buffers>(dev, reinterpret_cast<const std::vector<Vertex>&>(const_cast<std::vector<Vertex3D>&>(vertices)), indices))
    {

    }

    std::shared_ptr<Mesh> Mesh::MakeCube(c_device& dev)
    {
        using V = Vertex3D;
        std::vector<V> vertices = 
        {
            //front
            {{-0.5f,-0.5f, 0.5f}, {1,0,0}, {0,0}},
            {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {1,0}},
            {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {1,1}},
            {{-0.5f, 0.5f, 0.5f}, {1,0,0}, {0,1}},

            //back
            {{ 0.5f,-0.5f,-0.5f}, {0,1,0}, {0,0}},
            {{-0.5f,-0.5f,-0.5f}, {0,1,0}, {1,0}},
            {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {1,1}},
            {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {0,1}},

            //left
            {{-0.5f,-0.5f,-0.5f}, {0,0,1}, {0,0}},
            {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0}},
            {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {1,1}},
            {{-0.5f, 0.5f,-0.5f}, {0,0,1}, {0,1}},

            //right
            {{ 0.5f,-0.5f, 0.5f}, {1,1,0}, {0,0}},
            {{ 0.5f,-0.5f,-0.5f}, {1,1,0}, {1,0}},
            {{ 0.5f, 0.5f,-0.5f}, {1,1,0}, {1,1}},
            {{ 0.5f, 0.5f, 0.5f}, {1,1,0}, {0,1}},

            //top
            {{-0.5f, 0.5f, 0.5f}, {1,0,1}, {0,0}},
            {{ 0.5f, 0.5f, 0.5f}, {1,0,1}, {1,0}},
            {{ 0.5f, 0.5f,-0.5f}, {1,0,1}, {1,1}},
            {{-0.5f, 0.5f,-0.5f}, {1,0,1}, {0,1}},

            //bottom
            {{-0.5f,-0.5f,-0.5f}, {0,1,1}, {0,0}},
            {{ 0.5f,-0.5f,-0.5f}, {0,1,1}, {1,0}},
            {{ 0.5f,-0.5f, 0.5f}, {0,1,1}, {1,1}},
            {{-0.5f,-0.5f, 0.5f}, {0,1,1}, {0,1}},
        };

        std::vector<uint32_t> idecies = 
        {
            //front
            0, 1, 2,  2, 3, 0,
            //back
            4, 5, 6,  6, 7, 4,
            //left
            8, 9,10, 10,11, 8,
            //right
            12,13,14, 14,15,12,
            //top
            16,17,18, 18,19,16,
            //bottom
            20,21,22, 22,23,20
        };
        return std::make_shared<Mesh>(dev, vertices, idecies);
    }
}