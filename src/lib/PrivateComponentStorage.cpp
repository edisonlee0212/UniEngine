#include <EntityManager.hpp>
#include <PrivateComponentStorage.hpp>
using namespace UniEngine;
void PrivateComponentStorage::RemovePrivateComponent(Entity entity, size_t typeID)
{
    const auto search = m_pOwnersCollectionsMap.find(typeID);
    if (search != m_pOwnersCollectionsMap.end())
    {
        auto &collection = m_pOwnersCollectionsList[search->second].second;
        const auto entitySearch = collection->m_ownersMap.find(entity);
        if (entitySearch != collection->m_ownersMap.end())
        {
            if (entity != entitySearch->first)
            {
                UNIENGINE_ERROR("RemovePrivateComponent: Entity mismatch!");
                return;
            }
            if (collection->m_ownersList.size() == 1)
            {
                const auto eraseHash = typeID;
                const auto eraseIndex = search->second;
                const auto backHash = m_pOwnersCollectionsList.back().first;
                m_pOwnersCollectionsMap[backHash] = eraseIndex;
                std::swap(m_pOwnersCollectionsList[eraseIndex], m_pOwnersCollectionsList.back());
                m_pOwnersCollectionsMap.erase(eraseHash);
                m_pOwnersCollectionsList.pop_back();
            }
            else
            {
                const auto eraseIndex = entitySearch->second;
                const auto backEntity = collection->m_ownersList.back();
                collection->m_ownersMap[backEntity] = eraseIndex;
                collection->m_ownersMap.erase(entity);
                collection->m_ownersList[eraseIndex] = backEntity;
                collection->m_ownersList.pop_back();
            }
        }
    }
}

void PrivateComponentStorage::DeleteEntity(Entity entity)
{
    for (auto &element : EntityManager::GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements)
    {
        RemovePrivateComponent(entity, element.m_typeId);
    }
}

void PrivateComponentStorage::SetPrivateComponent(Entity entity, size_t id)
{
    const auto search = m_pOwnersCollectionsMap.find(id);
    if (search != m_pOwnersCollectionsMap.end())
    {
        const auto insearch = m_pOwnersCollectionsList[search->second].second->m_ownersMap.find(entity);
        if (insearch == m_pOwnersCollectionsList[search->second].second->m_ownersMap.end())
        {
            m_pOwnersCollectionsList[search->second].second->m_ownersMap.insert(
                {entity, m_pOwnersCollectionsList[search->second].second->m_ownersList.size()});
            m_pOwnersCollectionsList[search->second].second->m_ownersList.push_back(entity);
        }
    }
    else
    {
        std::unique_ptr<POwnersCollection> collection = std::make_unique<POwnersCollection>();
        collection->m_ownersMap.insert({entity, 0});
        collection->m_ownersList.push_back(entity);
        m_pOwnersCollectionsMap.insert({id, m_pOwnersCollectionsList.size()});
        m_pOwnersCollectionsList.push_back(std::make_pair(id, std::move(collection)));
    }
}
