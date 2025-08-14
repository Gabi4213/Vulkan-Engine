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
        explicit SceneGraph(ECSRegistry* inRegistry) : registry(inRegistry) {}

        void OnImGuiRender();

        Entity GetSelected() const { return selected; }
        void   SetSelected(Entity e) { selected = e; } 

        using TextureLoader = std::function<std::shared_ptr<Texture2D>(const std::string& path)>;
        void SetTextureLoader(TextureLoader loader, const std::string& assetRoot = "assets");

        ImVec2 getSceneViewportSize() const { return sceneViewportSize; }

    private:

        std::string MakeEntityLabel(Entity e);
        void DrawTexturePicker(SpriteRenderer& sr, const char* popupId);
        void BeginMainDockspace();

        ECSRegistry* registry = nullptr;
        Entity       selected = 0; // 0 = invalid


        TextureLoader loadTexture;
        std::filesystem::path assetRoot = "assets";
        std::filesystem::path browseDir = assetRoot;
        
        ImVec2 sceneViewportSize { 0,0 };
    };
}
