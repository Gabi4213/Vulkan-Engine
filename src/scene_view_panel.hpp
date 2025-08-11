#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"

namespace lavander 
{
    class SceneViewPanel
    {
    public:
        void OnImGuiRender();
        void SetSceneTexture(ImTextureID id) { sceneTex_ = id; }

    private:

        ImTextureID sceneTex_ = 0;

        //imguizmo camera state - will change later to actual cam
        bool useOrtho = true;
        float fovDeg = 45.0f;
        float orthoSize = 5.0f;
        float nearClip = 0.1f;
        float farClip = 100.0f;
        float gridSize = 100.0f;

        glm::mat4 view = glm::lookAt(glm::vec3(4, 4, 4), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
        glm::mat4 proj = glm::perspective(glm::radians(fovDeg), 16.0f / 9.0f, nearClip, farClip);
    };
}