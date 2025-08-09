// ecs_registry.hpp
#pragma once
#include "component_storage.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>

namespace lavander {

    class ECSRegistry {
    public:
        Entity createEntity()
        {
            Entity e = nextEntityId++;
            entities.push_back(e);
            alive.insert(e);
            return e;
        }

        //registers external id's / ensure it's in the list
        void ensureAlive(Entity e) 
        {
            if (!isAlive(e)) {
                entities.push_back(e);
                alive.insert(e);
            }
        }

        bool isAlive(Entity e) const 
        { 
            return alive.count(e) != 0;
        }

        void destroyEntity(Entity e)
        {
            if (!isAlive(e)) return;
            alive.erase(e);
            entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
            // NOTE: components are NOT removed automatically here.
            // You can call removeAllComponents<T>(e) for the types you use,
            // or add a type-erased storage system later.
        }

        const std::vector<Entity>& getAllEntities() const
        { 
            return entities; 
        }

        template<typename T>
        void addComponent(Entity entity, const T& component) 
        {
            ensureAlive(entity);
            getStorage<T>().add(entity, component);
        }

        template<typename T>
        std::vector<T>* getComponents(Entity entity)
        {
            return getStorage<T>().get(entity);
        }

        template<typename T>
        void removeComponent(Entity entity) 
        {
            getStorage<T>().removeComponent(entity);
        }

        template<typename T>
        void removeAllComponents(Entity entity)
        {
            getStorage<T>().removeAll(entity);
        }

        template<typename T>
        std::unordered_map<Entity, std::vector<T>>& getAllComponentsOfType() 
        {
            return getStorage<T>().getAll();
        }

    private:
        Entity nextEntityId = 1;
        std::vector<Entity> entities;
        std::unordered_set<Entity> alive;

        template<typename T>
        ComponentStorage<T>& getStorage() 
        {
            static ComponentStorage<T> storage;
            return storage;
        }
    };

}
