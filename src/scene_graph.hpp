#pragma once
#include "ecs_registry.hpp"
#include "components.hpp"
#include <string>
#include <functional>
#include <filesystem>

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

    private:
        std::string MakeEntityLabel(Entity e);
        void DrawTexturePicker(SpriteRenderer& sr, const char* popupId);

        ECSRegistry* m_Registry = nullptr;
        Entity       m_Selected = 0; // 0 = invalid for us


        TextureLoader m_LoadTexture;
        std::filesystem::path m_AssetRoot = "assets";
        std::filesystem::path m_BrowseDir = m_AssetRoot;
    };
}
