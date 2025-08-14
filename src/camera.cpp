#include "camera.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

// If you use ImGui: skip inputs when it wants capture
#include <imgui.h>

namespace lavander
{
    Camera::Camera() 
    {
        rebuildView();
        rebuildProjection();
    }

    void Camera::setPerspective(float fovDeg, float aspect, float nearPlane, float farPlane) 
    {
        ortho = false;
        fovDeg = fovDeg;
        aspect = std::max(0.0001f, aspect);
        near = nearPlane; far = farPlane;
        rebuildProjection();
    }

    void Camera::setOrthographic(float size, float aspect, float nearPlane, float farPlane)
    {
        ortho = true;
        orthoSize = std::max(0.0001f, size);
        aspect = std::max(0.0001f, aspect);
        near = nearPlane; far = farPlane;
        rebuildProjection();
    }

    void Camera::setAspect(float aspect) 
    {
        aspect = std::max(0.0001f, aspect);
        rebuildProjection();
    }

    void Camera::lookAt(const glm::vec3& target, const glm::vec3& inUp)
    {
        forward = glm::normalize(target - position);
        right = glm::normalize(glm::cross(forward, up));
        up = glm::normalize(glm::cross(right, forward));

        const glm::vec3 f = forward;
        pitchDeg = glm::degrees(std::asin(glm::clamp(f.y, -1.f, 1.f)));
        yawDeg = glm::degrees(std::atan2(f.z, f.x));
        rebuildView();
    }

    glm::mat4 Camera::projForAspect(float aspect) const
    {
        if (ortho) 
        {
            float half = orthoSize * 0.5f;
            return glm::ortho(-half * aspect, half * aspect, -half, half, near, far);
        }
        else 
        {
            return glm::perspective(glm::radians(fovDeg), aspect, near, far);
        }
    }

    void Camera::setYawPitch(float yawDeg, float pitchDeg)
    {
        yawDeg = yawDeg;
        pitchDeg = std::clamp(pitchDeg, -89.9f, 89.9f);
        updateVectors();
        rebuildView();
    }

    void Camera::update(GLFWwindow* window, float deltaTime) 
    {
        //drag to rotate (right mouse button)
        const int rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        if (rmb == GLFW_PRESS)
        {
            if (!rmbRotating) 
            {
                rmbRotating = true; 
                lastX = mx; 
                lastY = my; 
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            double dx = mx - lastX;
            double dy = my - lastY;
            lastX = mx; lastY = my;

            yawDeg += static_cast<float>(dx) * lookSensitivity;
            pitchDeg += static_cast<float>(dy) * lookSensitivity;
            pitchDeg = std::clamp(pitchDeg, -89.9f, 89.9f);
            updateVectors();
            rebuildView();
        }
        else if (rmbRotating) 
        {
            rmbRotating = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        //movement
        if (rmb == GLFW_PRESS) 
        {
            float speed = moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
            {
                speed *= 2.5f;
            }

            glm::vec3 move(0.f);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move += up;
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move -= up;

            if (glm::dot(move, move) > 0.f) 
            {
                position += glm::normalize(move) * speed * deltaTime;
                rebuildView();
            }
        }

        //mouse wheel zoom
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.f) 
        {
            if (ortho) 
            {
                orthoSize *= (wheel > 0.f ? 0.9f : 1.1f);
                orthoSize = std::max(0.01f, orthoSize);
                rebuildProjection();
            }
            else
            {
                position += forward * (wheel * 1.0f);
                rebuildView();
            }
        }
    }

    void Camera::updateVectors() 
    {
        const float yaw = glm::radians(yawDeg);
        const float pitch = glm::radians(pitchDeg);

        glm::vec3 f;
        f.x = std::cos(yaw) * std::cos(pitch);
        f.y = std::sin(pitch);
        f.z = std::sin(yaw) * std::cos(pitch);
        forward = glm::normalize(f);

        const glm::vec3 worldUp(0.f, 1.f, 0.f);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }

    void Camera::rebuildView()
    {
        view = glm::lookAt(position, position + forward, up);
    }

    void Camera::rebuildProjection()
    {
        if (ortho)
        {
            const float half = orthoSize * 0.5f;
            proj = glm::ortho(-half * aspect, half * aspect, -half, half, near, far);
        }
        else
        {
            proj = glm::perspective(glm::radians(fovDeg), aspect, near, far);
        }

        if (vulkanClipY)
        {
            proj[1][1] *= -1.f;
        }
    }
}