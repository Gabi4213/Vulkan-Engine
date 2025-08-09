// component_type_db.hpp
#pragma once
#include <functional>
#include <vector>
#include <string>
#include "ecs_registry.hpp"

namespace lavander 
{

    struct ComponentTypeInfo 
    {
        std::string name;
        std::function<void(ECSRegistry&, Entity)> addDefault;
        std::function<void(ECSRegistry&, Entity)> removeAll;
    };

    class ComponentTypeDB 
    {
    public:
        static ComponentTypeDB& Get() { static ComponentTypeDB db; return db; }

        void registerType(const std::string& name,
            std::function<void(ECSRegistry&, Entity)> addDefault) {
            types_.push_back({ name, std::move(addDefault) });
        }

        const std::vector<ComponentTypeInfo>& types() const { return types_; }

    private:
        std::vector<ComponentTypeInfo> types_;
    };

}
