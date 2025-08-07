#pragma once
#include "entity.hpp"

#include <map>
#include <unordered_map>

namespace lavander 
{
    template <typename T>
    class ComponentStorage
    {
    public:

        void add(Entity entity, const T& component)
        {
            storage[entity].push_back(component);
        }

        std::vector<T>* get(Entity entity)
        {
            auto it = storage.find(entity);
            return it != storage.end() ? &it->second : nullptr;
        }

        void removeComponent(Entity entity)
        {
            storage.erase(entity);
        }

        void removeAll(Entity entity)
        {
            storage.erase(entity);
        }

        std::unordered_map<Entity, std::vector<T>>& getAll()
        {
            return storage;
        }

    private:

        std::unordered_map<Entity, std::vector<T>> storage;
    };

}