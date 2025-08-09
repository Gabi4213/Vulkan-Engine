#pragma once
#include "ecs_registry.hpp"
#include "components.hpp"
#include <string>

namespace lavander 
{
    class SceneGraph 
    {
    public:
        explicit SceneGraph(ECSRegistry* registry) : m_Registry(registry) {}
        void OnImGuiRender();

        Entity GetSelected() const { return m_Selected; }
        void   SetSelected(Entity e) { m_Selected = e; }

    private:
        std::string MakeEntityLabel(Entity e);

        ECSRegistry* m_Registry = nullptr;
        Entity       m_Selected = 0; // 0 = invalid for us
    };
}
