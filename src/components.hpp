// components.hpp
#pragma once
#include <glm/glm.hpp>
#include "texture2d.hpp"
#include <string>
#include "mesh.hpp"

namespace lavander
{
    struct Transform
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct Tag
    {
        std::string name;
    };

    struct SpriteRenderer
    {
        glm::vec3 color = glm::vec3(1.0f);
        std::shared_ptr<Texture2D> texture;
    };
    
    struct MeshFilter 
    {
        std::shared_ptr<Mesh> mesh;
    };

    struct MeshRenderer3D {
        std::shared_ptr<Texture2D> texture;
        glm::vec3 color{ 1,1,1 };
    };

    struct PushConst
    {
        glm::mat4 model;
        glm::vec4 color;
    };
}
