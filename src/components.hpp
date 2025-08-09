// components.hpp
#pragma once
#include <glm/glm.hpp>
#include <string>

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
};

struct PushConst 
{
    glm::mat4 model;
    glm::vec4 color;
};
