#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"

#include "camera.hpp"

namespace lavander 
{
    class SceneViewPanel
    {
    public:
        void OnImGuiRender();
        void SetSceneTexture(ImTextureID id) { sceneTex = id; }

        //accessors for engine
        const glm::mat4& getView() const { return cam.getView(); }
        const glm::mat4& getProj() const { return cam.getProj(); }
        float getAspect() const { return aspect; }
        bool  isHovered() const { return hovered; }
        Camera& camera() { return cam; }

        void setSceneAspect(float a) { sceneAspect = a; }
        void setSceneTexture(ImTextureID id) { sceneTex = id; }


    private:
        ImTextureID sceneTex = 0;
        Camera cam;
        float  aspect = 16.f / 9.f;
        float sceneAspect = 16.0f / 9.0f;
        bool   hovered = false;   
        float gridSize = 100.0f;
    };
}