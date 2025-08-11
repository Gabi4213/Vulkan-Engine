#include "scene_graph.hpp"
#include "component_type_db.hpp"
#include <algorithm>
#include <cstring> 
#include <imgui.h>
#include <ImGuizmo.h>

namespace lavander
{

    template<typename... Ts>
    static void RemoveAllKnown(lavander::ECSRegistry& reg, Entity e)
    {
        (reg.removeAllComponents<Ts>(e), ...);
    }

    void SceneGraph::SetTextureLoader(TextureLoader loader, const std::string& assetRoot)
    {
        m_LoadTexture = std::move(loader);
        m_AssetRoot = std::filesystem::path(assetRoot);

        if (!std::filesystem::exists(m_AssetRoot))
        {
            std::error_code ec;
            std::filesystem::create_directories(m_AssetRoot, ec);
        }

        m_BrowseDir = m_AssetRoot;
    }

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

    void SceneGraph::DrawTexturePicker(SpriteRenderer& sr, const char* popupId)
    {
        if (ImGui::BeginPopup(popupId)) {
            ImGui::TextUnformatted("Pick a texture from assets/");
            ImGui::Separator();

            ImGui::Text("Path: %s", m_BrowseDir.string().c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Up")) 
            {
                if (m_BrowseDir != m_AssetRoot) 
                {
                    m_BrowseDir = m_BrowseDir.parent_path();
                }
            }

            //possible extensions for sprite files
            static const char* exts[] = { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".hdr", ".dds" };

            ImGui::BeginChild("##assets_list", ImVec2(0, 300), true);


            for (auto& entry : std::filesystem::directory_iterator(m_BrowseDir)) 
            {
                if (!entry.is_directory()) continue;
                std::string name = entry.path().filename().string();
                if (ImGui::Selectable((name + "/").c_str(), false)) 
                {
                    m_BrowseDir = entry.path();
                }
            }

            for (auto& entry : std::filesystem::directory_iterator(m_BrowseDir))
            {
                if (!entry.is_regular_file()) continue;

                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                bool ok = false;

                for (auto* e : exts) 
                { 
                    if (ext == e) 
                    { 
                        ok = true; break; 
                    } 
                }

                if (!ok) continue;

                std::string name = entry.path().filename().string();
                if (ImGui::Selectable(name.c_str(), false)) 
                {
                    if (m_LoadTexture) 
                    {
                        auto tex = m_LoadTexture(entry.path().string());
                        if (tex) sr.texture = std::move(tex);
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void SceneGraph::OnImGuiRender()
    {
        //scene hierarchy
        ImGui::Begin("Scene");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0.0f && avail.y > 0.0f)
        {
            sceneViewportSize_ = avail;
        }

        //create empty entities
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

            ImGuiTreeNodeFlags flags =ImGuiTreeNodeFlags_OpenOnArrow |ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

            if (m_Selected == e) flags |= ImGuiTreeNodeFlags_Selected;

            std::string label = MakeEntityLabel(e);
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)e, flags, "%s", label.c_str());

            if (ImGui::IsItemClicked()) m_Selected = e;

            //context menu per entity
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    RemoveAllKnown<Transform, SpriteRenderer, Tag>(*m_Registry, e);
                    m_Registry->destroyEntity(e);

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
                    continue;
                }

                if (ImGui::BeginMenu("Add Component"))
                {
                    for (const auto& info : ComponentTypeDB::Get().types())
                    {
                        if (ImGui::MenuItem(info.name.c_str()))
                        {
                            info.addDefault(*m_Registry, e);
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }

            if (opened) ImGui::TreePop();
        }

        ImGui::End();

        //properties
        ImGui::Begin("Properties");

        if (m_Selected != 0)
        {
            ImGui::Text("Entity: %u", m_Selected);

            //add compontent
            if (ImGui::Button("+ Add Component"))
            {
                ImGui::OpenPopup("add_component_popup");
            }

            if (ImGui::BeginPopup("add_component_popup"))
            {
                static char filter[64] = { 0 };
                ImGui::InputTextWithHint("##compfilter", "Search...", filter, sizeof(filter));

                std::string filtLower = filter;
                std::transform(filtLower.begin(), filtLower.end(), filtLower.begin(), ::tolower);

                for (const auto& info : ComponentTypeDB::Get().types())
                {
                    if (filter[0] != '\0')
                    {
                        std::string nameLower = info.name;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        if (nameLower.find(filtLower) == std::string::npos) continue;
                    }

                    if (ImGui::MenuItem(info.name.c_str()))
                    {
                        info.addDefault(*m_Registry, m_Selected);
                    }
                }
                ImGui::EndPopup();
            }

            //tag
            if (auto* tags = m_Registry->getComponents<Tag>(m_Selected); tags && !tags->empty())
            {
                std::string& name = (*tags)[0].name;
                char buf[128];
                std::strncpy(buf, name.c_str(), sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';

                if (ImGui::InputText("Tag", buf, sizeof(buf)))
                {
                    name = buf;
                }

                ImGui::SameLine();

                if (ImGui::SmallButton("Remove Tag"))
                {
                    m_Registry->removeComponentAt<Tag>(m_Selected, 0);
                }
            }

            //transform component
            if (auto* trs = m_Registry->getComponents<Transform>(m_Selected))
            {
                for (int ti = static_cast<int>(trs->size()) - 1; ti >= 0; --ti)
                {
                    //refresh poiunter after removal
                    trs = m_Registry->getComponents<Transform>(m_Selected);

                    if (!trs || ti >= static_cast<int>(trs->size())) continue;

                    ImGui::PushID(ti);
                    std::string hdr = "Transform " + std::to_string(ti);
                    ImGui::SeparatorText(hdr.c_str());

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        m_Registry->removeComponentAt<Transform>(m_Selected, static_cast<size_t>(ti));
                        ImGui::PopID();
                        continue;
                    }

                    Transform& t = (*trs)[ti];
                    ImGui::DragFloat3("Position", &t.position.x, 0.05f);
                    ImGui::DragFloat3("Rotation (rad)", &t.rotation.x, 0.05f);
                    ImGui::DragFloat3("Scale", &t.scale.x, 0.05f, 0.01f, 100.0f);
                    ImGui::PopID();
                }
            }

            //sprite renderer
            if (auto* srs = m_Registry->getComponents<SpriteRenderer>(m_Selected)) 
            {
                for (int si = int(srs->size()) - 1; si >= 0; --si)
                {
                    srs = m_Registry->getComponents<SpriteRenderer>(m_Selected);

                    if (!srs || si >= int(srs->size())) continue;

                    ImGui::PushID(10000 + si);
                    std::string hdr = "SpriteRenderer " + std::to_string(si);
                    ImGui::SeparatorText(hdr.c_str());

                    //remvoe button
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        m_Registry->removeComponentAt<SpriteRenderer>(m_Selected, size_t(si));
                        ImGui::PopID();
                        continue;
                    }

                    SpriteRenderer& sr = (*srs)[si];

                    ImGui::ColorEdit3("Color", &sr.color.x);

                    //texture ui
                    const char* texLabel = (sr.texture ? "Texture: (set)" : "Texture: <None>");
                    ImGui::TextUnformatted(texLabel);
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear Texture"))
                    {
                        sr.texture.reset();
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Browse..."))
                    {
                        ImGui::OpenPopup("pick_tex_popup");
                    }

                    //asset browser pop up
                    DrawTexturePicker(sr, "pick_tex_popup");

                    ImGui::PopID();
                }
            }
        }
        ImGui::End();
    }
}