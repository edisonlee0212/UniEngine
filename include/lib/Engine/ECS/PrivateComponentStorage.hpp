#pragma once
#include "Entity.hpp"
#include <utility>
#include "Serialization.hpp"
namespace UniEngine
{
struct UNIENGINE_API POwnersCollection
{
    std::unordered_map<Entity, size_t, Entity> m_ownersMap;
    std::vector<Entity> m_ownersList;
    POwnersCollection()
    {
        m_ownersList = std::vector<Entity>();
        m_ownersMap = std::unordered_map<Entity, size_t, Entity>();
    }
};
class Scene;
class UNIENGINE_API PrivateComponentStorage
{
    std::unordered_map<size_t, size_t> m_pOwnersCollectionsMap;
    std::vector<std::pair<size_t, POwnersCollection>> m_pOwnersCollectionsList;
    std::unordered_map<size_t, std::vector<std::shared_ptr<IPrivateComponent>>> m_privateComponentPool;
  public:
    std::weak_ptr<Scene> m_scene;
    void RemovePrivateComponent(Entity entity, size_t typeID, const std::shared_ptr<IPrivateComponent> &privateComponent);
    void DeleteEntity(Entity entity);
    template <typename T = IPrivateComponent> std::shared_ptr<T> GetOrSetPrivateComponent(Entity entity);
    void SetPrivateComponent(Entity entity, size_t id);
    template <typename T = IPrivateComponent> void RemovePrivateComponent(Entity entity, const std::shared_ptr<IPrivateComponent> &privateComponent);
    template <typename T> const std::vector<Entity> *UnsafeGetOwnersList();
    template <typename T> const std::vector<Entity> GetOwnersList();
};

template <typename T> std::shared_ptr<T> PrivateComponentStorage::GetOrSetPrivateComponent(Entity entity)
{
    size_t id = typeid(T).hash_code();
    auto search = m_pOwnersCollectionsMap.find(id);
    if (search != m_pOwnersCollectionsMap.end())
    {
        const auto insearch = m_pOwnersCollectionsList[search->second].second.m_ownersMap.find(entity);
        if (insearch == m_pOwnersCollectionsList[search->second].second.m_ownersMap.end())
        {
            m_pOwnersCollectionsList[search->second].second.m_ownersMap.insert(
                {entity, m_pOwnersCollectionsList[search->second].second.m_ownersList.size()});
            m_pOwnersCollectionsList[search->second].second.m_ownersList.push_back(entity);
        }
    }
    else
    {
        POwnersCollection collection;
        collection.m_ownersMap.insert({entity, 0});
        collection.m_ownersList.push_back(entity);
        m_pOwnersCollectionsMap.insert({id, m_pOwnersCollectionsList.size()});
        m_pOwnersCollectionsList.emplace_back(id, std::move(collection));
    }
    const auto pSearch = m_privateComponentPool.find(id);
    if(pSearch != m_privateComponentPool.end() && !pSearch->second.empty()){
        auto back = pSearch->second.back();
        pSearch->second.pop_back();
        back->m_handle = Handle();
        return std::dynamic_pointer_cast<T>(back);
    }
    return Serialization::ProduceSerializable<T>();
}

template <typename T> void PrivateComponentStorage::RemovePrivateComponent(Entity entity, const std::shared_ptr<IPrivateComponent> &privateComponent)
{
    RemovePrivateComponent(entity, typeid(T).hash_code(), privateComponent);
}

template <typename T> const std::vector<Entity> *PrivateComponentStorage::UnsafeGetOwnersList()
{
    auto search = m_pOwnersCollectionsMap.find(typeid(T).hash_code());
    if (search != m_pOwnersCollectionsMap.end())
    {
        return &m_pOwnersCollectionsList[search->second].second.m_ownersList;
    }
    return nullptr;
}
} // namespace UniEngine
