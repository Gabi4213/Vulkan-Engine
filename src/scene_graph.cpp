#include "scene_graph.hpp"
#include <algorithm>
#include <cstring> 
#include <imgui.h>

namespace lavander
{
    std::string SceneGraph::MakeEntityLabel(Entity e)
    {
        std::string label = "Entity " + std::to_string(e);
        std::vector<Tag>* tags = m_Registry->getComponents<Tag>(e);

        if (tags && !tags->empty() && !(*tags)[0].name.empty())
        {
            label = (*tags)[0].name + " (" + std::to_string(e) + ")";
        }

        return label;
    }

    void SceneGraph::OnImGuiRender()
    {
        ImGui::Begin("Scene");

        // right click to add new entity
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity")) 
            {
                Entity e = m_Registry->createEntity();
                m_Registry->addComponent<Tag>(e, Tag{ "Empty" });
                m_Registry->addComponent<Transform>(e, Transform{});
                m_Selected = e;
            }
            ImGui::EndPopup();
        }

        const std::vector<Entity>& all = m_Registry->getAllEntities();
        for (size_t i = 0; i < all.size(); ++i)
        {
            Entity e = all[i];

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

            if (m_Selected == e) flags |= ImGuiTreeNodeFlags_Selected;

            std::string label = MakeEntityLabel(e);
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)e, flags, "%s", label.c_str());

            if (ImGui::IsItemClicked()) m_Selected = e;

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    m_Registry->destroyEntity(e);
                    if (m_Selected == e) 
                    {
                        m_Selected = 0;
                    }

                    ImGui::EndPopup();

                    if (opened) 
                    { 
                        ImGui::TreePop();
                    }

                    //entity removed
                    continue;
                }
                if (ImGui::BeginMenu("Add Component"))
                {
                    if (ImGui::MenuItem("Transform"))      m_Registry->addComponent<Transform>(e, Transform{});
                    if (ImGui::MenuItem("SpriteRenderer")) m_Registry->addComponent<SpriteRenderer>(e, SpriteRenderer{});
                    if (ImGui::MenuItem("Tag"))            m_Registry->addComponent<Tag>(e, Tag{ "New Tag" });
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            if (opened) ImGui::TreePop();
        }

        ImGui::End();

        //Properties
        ImGui::Begin("Properties");
        if (m_Selected != 0) 
        {
            ImGui::Text("Entity: %u", m_Selected);

            //tag
            std::vector<Tag>* tags = m_Registry->getComponents<Tag>(m_Selected);
            if (tags && !tags->empty()) 
            {
                std::string& name = (*tags)[0].name;
                char buf[128];
                std::strncpy(buf, name.c_str(), sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';

                if (ImGui::InputText("Tag", buf, sizeof(buf)))
                {
                    name = buf;
                }
            }

            //transform
            std::vector<Transform>* trs = m_Registry->getComponents<Transform>(m_Selected);
            if (trs)
            {
                for (size_t ti = 0; ti < trs->size(); ++ti)
                {
                    Transform& t = (*trs)[ti];
                    std::string hdr = "Transform " + std::to_string(ti);
                    ImGui::SeparatorText(hdr.c_str());
                    ImGui::DragFloat3("Position", &t.position.x, 0.05f);
                    ImGui::DragFloat3("Rotation (rad)", &t.rotation.x, 0.05f);
                    ImGui::DragFloat3("Scale", &t.scale.x, 0.05f, 0.01f, 100.0f);
                }
            }

            //sprite renderer
            std::vector<SpriteRenderer>* srs = m_Registry->getComponents<SpriteRenderer>(m_Selected);
            if (srs)
            {
                for (size_t si = 0; si < srs->size(); ++si)
                {
                    SpriteRenderer& sr = (*srs)[si];
                    std::string hdr = "SpriteRenderer " + std::to_string(si);
                    ImGui::SeparatorText(hdr.c_str());
                    ImGui::ColorEdit3("Color", &sr.color.x);
                }
            }
        }
        ImGui::End();
    }
}