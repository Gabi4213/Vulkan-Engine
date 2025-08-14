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

    //toolbar for gizmo so transform, rotate and scale plus snap
    {
        ImVec2 toolbarPos = ImVec2(contentPos.x + 8, contentPos.y + 8);
        ImGui::SetCursorScreenPos(toolbarPos);
        ImGui::BeginChild("##GizmoToolbar", ImVec2(320, 28), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        bool tSel = (gizmoOp == GizmoOp::Translate);
        bool rSel = (gizmoOp == GizmoOp::Rotate);
        bool sSel = (gizmoOp == GizmoOp::Scale);

        if (ImGui::Selectable("T", tSel, 0, ImVec2(22, 22))) 
        {
            gizmoOp = GizmoOp::Translate;
        }
        ImGui::SameLine();

        if (ImGui::Selectable("R", rSel, 0, ImVec2(22, 22))) 
        { 
            gizmoOp = GizmoOp::Rotate;
        }
        ImGui::SameLine();

        if (ImGui::Selectable("S", sSel, 0, ImVec2(22, 22))) 
        { 
            gizmoOp = GizmoOp::Scale; 
        }
        ImGui::SameLine(0, 12);

        ImGui::Checkbox("Snap", &useSnap);
        ImGui::SameLine();

        if (useSnap)
        {
            if (gizmoOp == GizmoOp::Rotate)
            {
                ImGui::SetNextItemWidth(80);
                ImGui::DragFloat("##rotSnap", &snapRotateDeg, 1.f, 1.f, 90.f, "%.0f°");
            }
            else if (gizmoOp == GizmoOp::Translate) 
            {
                ImGui::SetNextItemWidth(140);
                ImGui::DragFloat3("##moveSnap", &snapMove.x, 0.1f);
            }
            else
            {
                ImGui::SetNextItemWidth(140);
                ImGui::DragFloat3("##scaleSnap", &snapScale.x, 0.05f, 0.01f);
            }
        }

        ImGui::EndChild();
    }

    //uv cropping for aspect ration regardless of scene view size
    const float panelAspect = contentSize.x / std::max(1.0f, contentSize.y);
    cam.setAspect(sceneAspect);

    ImVec2 uv0(0, 0), uv1(1, 1);
    if (panelAspect > sceneAspect)
    {
        float keep = sceneAspect / panelAspect;
        float pad = (1.0f - keep) * 0.5f;
        uv0.y = pad; uv1.y = 1.0f - pad;
    }
    else if (panelAspect < sceneAspect)
    {
        float keep = panelAspect / sceneAspect;
        float pad = (1.0f - keep) * 0.5f;
        uv0.x = pad; uv1.x = 1.0f - pad;
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

    //grid
    static glm::mat4 I(1.0f);
    ImGuizmo::DrawGrid(&viewM[0][0], &projOverlay[0][0], &I[0][0], gridSize);

    //view cube
    const float cubeSize = 96.0f;
    ImVec2 cubePos = ImVec2(contentPos.x + 10.0f, contentPos.y + 10.0f);
    ImGuizmo::ViewManipulate(&viewM[0][0], 4.0f, cubePos, ImVec2(cubeSize, cubeSize), 0x10101010);

    //camera for cube
    glm::mat4 inv = glm::inverse(viewM);
    glm::vec3 fwd = glm::normalize(-glm::vec3(inv[2]));
    glm::vec3 camPos3 = glm::vec3(inv[3]);
    float yawDeg = glm::degrees(std::atan2(fwd.z, fwd.x));
    float pitchDeg = glm::degrees(std::asin(glm::clamp(fwd.y, -1.f, 1.f)));
    cam.setPosition(camPos3);
    cam.setYawPitch(yawDeg, pitchDeg);

    viewM = cam.getView();
    projOverlay = cam.getProj();

    if (panelAspect > sceneAspect)
    {
        float k = panelAspect / sceneAspect; projOverlay[1][1] *= k;
    }
    else if (panelAspect < sceneAspect) 
    {
        float k = sceneAspect / panelAspect; projOverlay[0][0] *= k;
    }
    
    //gizmo for entities
    if (registry_ && selectedEntity_ != 0) 
    {
        if (auto* trs = registry_->getComponents<Transform>(selectedEntity_))
        {
            if (!trs->empty())
            {
                Transform& t = (*trs)[0];
                glm::mat4 model = BuildTRS(t);

                ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;

                if (gizmoOp == GizmoOp::Rotate)
                { 
                    op = ImGuizmo::ROTATE;
                }
                else if (gizmoOp == GizmoOp::Scale) 
                { 
                    op = ImGuizmo::SCALE;
                }

                ImGuizmo::MODE mode = ImGuizmo::LOCAL;

                float* snapPtr = nullptr;
                float snapVals[3];

                if (useSnap)
                {
                    if (op == ImGuizmo::TRANSLATE) { snapVals[0] = snapMove.x;  snapVals[1] = snapMove.y;  snapVals[2] = snapMove.z;  snapPtr = snapVals; }
                    else if (op == ImGuizmo::SCALE) { snapVals[0] = snapScale.x; snapVals[1] = snapScale.y; snapVals[2] = snapScale.z; snapPtr = snapVals; }
                    else { snapVals[0] = snapRotateDeg; snapPtr = snapVals; }
                }

                ImGuizmo::Manipulate(&viewM[0][0], &projOverlay[0][0], op, mode, &model[0][0], nullptr, snapPtr);

                if (ImGuizmo::IsUsing()) 
                {
                    float tPos[3], tRotDeg[3], tScl[3];
                    ImGuizmo::DecomposeMatrixToComponents(&model[0][0], tPos, tRotDeg, tScl);
                    t.position = { tPos[0], tPos[1], tPos[2] };
                    t.rotation = glm::radians(glm::vec3(tRotDeg[0], tRotDeg[1], tRotDeg[2]));
                    t.scale = { tScl[0], tScl[1], tScl[2] };
                }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void lavander::SceneViewPanel::SetContext(ECSRegistry* reg, Entity selected)
{
    registry_ = reg;
    selectedEntity_ = selected;
}