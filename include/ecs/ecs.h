#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <stdint.h>

namespace ecs
{
    using EntityID = uint32_t;

    struct IPool
    {
    public:
        virtual ~IPool() = default;
        virtual void Remove(EntityID id) = 0;
        virtual bool Has(EntityID id) = 0;
    };

    template <typename T>
    struct Pool : public IPool
    {
    public:
        std::vector<T> data;                                // Dense array of actual data
        std::vector<EntityID> denseToEntity;                // Tells us: "Who owns Index 5?"
        std::unordered_map<EntityID, size_t> entityToIndex; // Tells us: "Where is Entity 42?"

        void Add(EntityID id, T component)
        {
            if (Has(id))
            {
                data[entityToIndex[id]] = component;
                return;
            }
            entityToIndex[id] = data.size();
            denseToEntity.push_back(id);
            data.push_back(component);
        }

        void Remove(EntityID id) override
        {
            if (entityToIndex.find(id) == entityToIndex.end())
                return;

            size_t indexToRemove = entityToIndex[id];
            size_t lastIndex = data.size() - 1;
            EntityID lastEntity = denseToEntity[lastIndex];

            // 1. Swap the data: Move the last element into the hole
            data[indexToRemove] = std::move(data[lastIndex]);
            denseToEntity[indexToRemove] = lastEntity;

            // 2. Update the mapping for the entity that just moved
            entityToIndex[lastEntity] = indexToRemove;

            // 3. Pop the last (now duplicate) elements
            data.pop_back();
            denseToEntity.pop_back();
            entityToIndex.erase(id);
        }

        bool Has(EntityID id) override
        {
            return entityToIndex.find(id) != entityToIndex.end();
        }

        T &Get(EntityID id) { return data[entityToIndex[id]]; }
    };

    class World
    {
    private:
        EntityID nextEntity = 0;
        // Maps a Component Type to its specific Pool
        std::unordered_map<std::type_index, std::shared_ptr<IPool>> pools;

    public:
        EntityID CreateEntity() { return nextEntity++; }

        template <typename T>
        Pool<T> *GetPool()
        {
            auto type = std::type_index(typeid(T));
            if (pools.find(type) == pools.end())
            {
                pools[type] = std::make_shared<Pool<T>>();
            }
            return static_cast<Pool<T> *>(pools[type].get());
        }

        template <typename T>
        bool HasComponent(EntityID id)
        {
            auto type = std::type_index(typeid(T));
            auto it = pools.find(type);
            if (it == pools.end())
                return false;

            auto *pool = static_cast<Pool<T> *>(it->second.get());
            return pool->Has(id);
        }

        void RemoveEntity(EntityID id)
        {
            for (auto const &[type, pool] : pools)
            {
                pool->Remove(id);
            }
        }

        template <typename T>
        void AddComponent(EntityID id, T component)
        {
            GetPool<T>()->Add(id, component);
        }

        template <typename T>
        T &GetComponent(EntityID id)
        {
            return GetPool<T>()->Get(id);
        }
    };

    struct Active
    {
        bool IsActive = true;
    };

    struct Entity
    {
        EntityID id;
        World *world;

        template <typename T>
        Entity &Add(T component)
        {
            world->AddComponent<T>(id, component);
            return *this;
        }

        template <typename T>
        T &Get() { return world->GetComponent<T>(id); }

        template <typename T>
        bool Has() { return world->HasComponent<T>(id); }

        void SetActive(bool state)
        {
            if (!world->HasComponent<Active>(id))
                world->AddComponent<Active>(id, {state});
            else
                world->GetComponent<Active>(id).IsActive = state;
        }
    };
}
