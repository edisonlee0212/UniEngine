#pragma once
#include <Entity.hpp>
namespace UniEngine
{
struct POwnersCollection
{
    std::unordered_map<Entity, size_t, Entity> m_ownersMap;
    std::vector<Entity> m_ownersList;
    POwnersCollection()
    {
        m_ownersList = std::vector<Entity>();
        m_ownersMap = std::unordered_map<Entity, size_t, Entity>();
    }
};
class PrivateComponentStorage
{
    std::unordered_map<std::size_t, size_t> m_pOwnersCollectionsMap;
    std::vector<std::pair<size_t, POwnersCollection>> m_pOwnersCollectionsList;

  public:
    UNIENGINE_API void RemovePrivateComponent(Entity entity, size_t typeID);
    void DeleteEntity(Entity entity);
    template <typename T = IPrivateComponent> void SetPrivateComponent(Entity entity);
    void SetPrivateComponent(Entity entity, size_t id);
    template <typename T = IPrivateComponent> void RemovePrivateComponent(Entity entity);
    template <typename T> const std::vector<Entity> *UnsafeGetOwnersList();
    template <typename T> const std::vector<Entity> GetOwnersList();
};

template <typename T> void PrivateComponentStorage::SetPrivateComponent(Entity entity)
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
        m_pOwnersCollectionsList.push_back(std::make_pair(id, std::move(collection)));
    }
}

template <typename T> void PrivateComponentStorage::RemovePrivateComponent(Entity entity)
{
    RemovePrivateComponent(entity, typeid(T).hash_code());
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
