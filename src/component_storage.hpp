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

        bool removeAt(Entity entity, size_t index)
        {
            auto it = storage.find(entity);
            if (it == storage.end()) return false;
            auto& vec = it->second;
            if (index >= vec.size()) return false;
            vec.erase(vec.begin() + static_cast<long>(index));
            if (vec.empty()) storage.erase(it);
            return true;
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