#include "scene_view_panel.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

using namespace lavander;

void SceneViewPanel::OnImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene ###SceneView", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 minR = ImGui::GetWindowContentRegionMin();
    ImVec2 maxR = ImGui::GetWindowContentRegionMax();
    ImVec2 contentPos = { winPos.x + minR.x, winPos.y + minR.y };
    ImVec2 contentSize = { std::max(1.0f, maxR.x - minR.x), std::max(1.0f, maxR.y - minR.y) };

    hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    const float panelAspect = contentSize.x / std::max(1.0f, contentSize.y);
    cam.setAspect(sceneAspect);

    //crop UVs to fill the window area 
    ImVec2 uv0(0, 0), uv1(1, 1);
    if (panelAspect > sceneAspect)
    {
        float keep = sceneAspect / panelAspect;
        float pad = (1.0f - keep) * 0.5f;
        uv0.y = pad;
        uv1.y = 1.0f - pad;
    }
    else if (panelAspect < sceneAspect) 
    {
        float keep = panelAspect / sceneAspect;
        float pad = (1.0f - keep) * 0.5f;
        uv0.x = pad;
        uv1.x = 1.0f - pad;
    }

    if (sceneTex != 0) 
    {
        ImGui::SetCursorScreenPos(contentPos);
        ImGui::Image(sceneTex, contentSize, uv0, uv1);
    }

    glm::mat4 viewM = cam.getView();
    glm::mat4 projOverlay = cam.getProj();

    if (panelAspect > sceneAspect) 
    {
        float k = panelAspect / sceneAspect;
        projOverlay[1][1] *= k;
    }
    else if (panelAspect < sceneAspect) 
    {
        float k = sceneAspect / panelAspect;
        projOverlay[0][0] *= k;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(dl);
    ImGuizmo::SetOrthographic(cam.isOrtho());
    ImGuizmo::SetRect(contentPos.x, contentPos.y, contentSize.x, contentSize.y);

    static glm::mat4 I(1.0f);
    ImGuizmo::DrawGrid(&viewM[0][0], &projOverlay[0][0], &I[0][0], gridSize);

    //view cube
    const float cubeSize = 96.0f;
    ImVec2 cubePos = ImVec2(contentPos.x + 10.0f, contentPos.y + 10.0f);
    ImGuizmo::ViewManipulate(&viewM[0][0], 4.0f, cubePos, ImVec2(cubeSize, cubeSize), 0x10101010);

    //camera stuff
    glm::mat4 inv = glm::inverse(viewM);
    glm::vec3 fwd = glm::normalize(-glm::vec3(inv[2]));
    glm::vec3 camPos3 = glm::vec3(inv[3]);
    float yawDeg = glm::degrees(std::atan2(fwd.z, fwd.x));
    float pitchDeg = glm::degrees(std::asin(glm::clamp(fwd.y, -1.f, 1.f)));
    cam.setPosition(camPos3);
    cam.setYawPitch(yawDeg, pitchDeg);

    ImGui::End();
    ImGui::PopStyleVar();
}