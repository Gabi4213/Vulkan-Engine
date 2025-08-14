#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

namespace lavander 
{
    class Camera
    {
    public:

        Camera();

        //setters
        void setPerspective(float fovDeg, float aspect, float nearPlane, float farPlane);
        void setOrthographic(float size, float aspect, float nearPlane, float farPlane);
        void setAspect(float aspect);
        void setYawPitch(float yawDeg, float pitchDeg);

        void setVulkanClipY(bool flip) { vulkanClipY = flip; rebuildProjection(); }
        void setPosition(const glm::vec3& p) { position = p; rebuildView(); }
        void setOrthoSize(float s) { orthoSize = s; rebuildProjection(); }
        void setMoveSpeed(float s) { moveSpeed = s; }
        void setLookSensitivity(float s) { lookSensitivity = s; }
        void setFovDegrees(float inFovDeg) { fovDeg = inFovDeg; }
        void setNearClip(float inNear) { near = inNear; }
        void setFarClip(float inFar) { far = inFar; }
        void setOrtho(bool inOrtho) { ortho = inOrtho; }

        //getters
        bool isOrtho() const { return ortho; }
        float getFov() const { return fovDeg; }
        float getOrthoSize() const { return orthoSize; }
        float getFovDegrees() const { return fovDeg; }
        float getNearClip() const { return near; }
        float getFarClip() const { return far; }
        const glm::mat4& getView() const { return view; }
        const glm::mat4& getProj() const { return proj; }
        glm::mat4 getViewProj() const { return proj * view; }

        void update(GLFWwindow* window, float deltaTime);
        void lookAt(const glm::vec3& target, const glm::vec3& inUp = { 0,1,0 });
        glm::mat4 projForAspect(float aspect) const;

    private:
        void rebuildProjection();
        void rebuildView();
        void updateVectors();

        glm::vec3 position { 0.0f, 0.0f, 5.0f };
        float yawDeg = -90.0f;
        float pitchDeg = 0.0f;

        glm::vec3 forward { 0.0f, 0.0f, -1.0f };
        glm::vec3 right { 1.0f, 0.0f,  0.0f };
        glm::vec3 up { 0.0f, 1.0f,  0.0f };

        //camera settings
        bool ortho = false;
        float aspect = 16.0f / 9.0f;
        float fovDeg = 45.0f;
        float orthoSize = 10.f;
        float near = 0.1f;
        float far = 1000.0f;
        bool vulkanClipY = true;

        //controls/input stuff
        float moveSpeed = 6.0f;
        float lookSensitivity = 0.12f;
        bool rmbRotating = false;
        double lastX = 0.0, lastY = 0.0;

        //matricies chached here
        glm::mat4 view { 1.0f };
        glm::mat4 proj { 1.0f };
    };
}