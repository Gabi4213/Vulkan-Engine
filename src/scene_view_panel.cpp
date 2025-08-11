#include "scene_view_panel.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

using namespace lavander;

void SceneViewPanel::OnImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 minR = ImGui::GetWindowContentRegionMin();
    ImVec2 maxR = ImGui::GetWindowContentRegionMax();
    ImVec2 pos = ImVec2(winPos.x + minR.x, winPos.y + minR.y);
    ImVec2 size = ImVec2(std::max(1.0f, maxR.x - minR.x), std::max(1.0f, maxR.y - minR.y));

    float aspect = size.x / std::max(1.0f, size.y);

    if (useOrtho) 
    {
        float half = orthoSize * 0.5f;
        proj = glm::ortho(-half * aspect, half * aspect, -half, half, nearClip, farClip);
    }
    else 
    {
        proj = glm::perspective(glm::radians(fovDeg), aspect, nearClip, farClip);
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->ChannelsSplit(3);

    //scene image
    dl->ChannelsSetCurrent(0);
    if (sceneTex_ != 0) {
        ImGui::SetCursorScreenPos(pos);
        ImGui::Image(sceneTex_, size, ImVec2(0, 0), ImVec2(1, 1));
    }

    //grid
    dl->ChannelsSetCurrent(1);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(useOrtho);
    ImGuizmo::SetDrawlist(dl);
    ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
    glm::mat4 I(1.0f);
    ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], &I[0][0], gridSize);

    //gizmo
    dl->ChannelsSetCurrent(2);
    ImGuizmo::SetDrawlist(dl);
    ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
    const float viewCubeSize = 96.0f;
    ImVec2 cubePos = ImVec2(pos.x + 10.0f, pos.y + 10.0f);
    ImGuizmo::ViewManipulate(&view[0][0], 4.0f, cubePos, ImVec2(viewCubeSize, viewCubeSize), 0x10101010);

    dl->ChannelsMerge();

    //controls temp
    ImGui::SetCursorScreenPos(ImVec2(pos.x + 8, pos.y + 8 + viewCubeSize + 8));
    ImGui::BeginChild("SceneViewControls", ImVec2(280, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::Checkbox("Orthographic", &useOrtho);

    if (useOrtho) 
    {
        ImGui::SliderFloat("Ortho Size", &orthoSize, 0.5f, 50.0f); 
    }
    else
    { 
        ImGui::SliderFloat("FOV", &fovDeg, 10.0f, 90.0f);
    }

    ImGui::SliderFloat("Near", &nearClip, 0.01f, 5.0f);
    ImGui::SliderFloat("Far", &farClip, 10.0f, 500.0f);
    ImGui::SliderFloat("Grid Size", &gridSize, 0.1f, 10.0f);
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
}

