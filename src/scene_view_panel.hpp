#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "ecs_registry.hpp"
#include "components.hpp"

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
        void SetContext(ECSRegistry* reg, Entity selected);

    private:

        ECSRegistry* registry_ = nullptr;
        Entity       selectedEntity_ = 0;

        ImTextureID sceneTex = 0;
        Camera cam;
        float aspect = 16.f / 9.f;
        float sceneAspect = 16.0f / 9.0f;
        bool hovered = false;   
        float gridSize = 100.0f;

        enum class GizmoOp { Translate, Rotate, Scale } gizmoOp = GizmoOp::Translate;
        bool useSnap = false;
        glm::vec3 snapMove { 1.0f,1.0f,1.0f };
        glm::vec3 snapScale { 0.1f,0.1f,0.1f };
        float snapRotateDeg = 90.0f;

        static glm::mat4 BuildTRS(const Transform& t)
        {
            glm::mat4 m(1.0f);
            m = glm::translate(m, t.position);
            m = m * glm::rotate(glm::mat4(1.0f), t.rotation.z, glm::vec3(0, 0, 1)) * glm::rotate(glm::mat4(1.0f), t.rotation.y, glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1.0f), t.rotation.x, glm::vec3(1, 0, 0));
            m = glm::scale(m, t.scale);
            return m;
        }
    };
}