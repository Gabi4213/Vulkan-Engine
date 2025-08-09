// component_type_db_init.hpp (or in Engine.cpp near ctor)
#pragma once
#include "component_type_db.hpp"
#include "components.hpp"

namespace lavander
{
    inline void RegisterBuiltInComponents() 
    {
        auto& db = ComponentTypeDB::Get();

        db.registerType("Transform", [](ECSRegistry& r, Entity e) 
            {
                r.addComponent<Transform>(e, Transform{});
            });

        db.registerType("SpriteRenderer", [](ECSRegistry& r, Entity e)
            {
                r.addComponent<SpriteRenderer>(e, SpriteRenderer{ glm::vec3(1,1,1) });
            });

        db.registerType("Tag", [](ECSRegistry& r, Entity e)
            {
                r.addComponent<Tag>(e, Tag{ "New Tag" });
            });
    }
}
