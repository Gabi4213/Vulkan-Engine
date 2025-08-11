#pragma once
#include "ecs_registry.hpp"
#include "components.hpp"
#include <string>
#include <functional>
#include <filesystem>

#include <imgui.h>

namespace lavander 
{
    class SceneGraph 
    {
    public:
        explicit SceneGraph(ECSRegistry* registry) : m_Registry(registry) {}

        void OnImGuiRender();

        Entity GetSelected() const { return m_Selected; }
        void   SetSelected(Entity e) { m_Selected = e; }

        using TextureLoader = std::function<std::shared_ptr<Texture2D>(const std::string& path)>;
        void SetTextureLoader(TextureLoader loader, const std::string& assetRoot = "assets");

        ImVec2 getSceneViewportSize() const { return sceneViewportSize_; }

    private:

        std::string MakeEntityLabel(Entity e);
        void DrawTexturePicker(SpriteRenderer& sr, const char* popupId);
        void BeginMainDockspace();

        ECSRegistry* m_Registry = nullptr;
        Entity       m_Selected = 0; // 0 = invalid


        TextureLoader m_LoadTexture;
        std::filesystem::path m_AssetRoot = "assets";
        std::filesystem::path m_BrowseDir = m_AssetRoot;
        
        ImVec2 sceneViewportSize_{ 0,0 };
    };
}
