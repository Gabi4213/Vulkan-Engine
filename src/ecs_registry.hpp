#pragma once
#include "component_storage.hpp"

namespace lavander
{
    class ECSRegistry
    {
    public:

        Entity createEntity()
        {
            return nextEntityId++;
        }

        template<typename T>
        void addComponent(Entity entity, const T& component)
        {
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

        template<typename T>
        ComponentStorage<T>& getStorage() 
        {
            static ComponentStorage<T> storage;
            return storage;
        }
    };

}