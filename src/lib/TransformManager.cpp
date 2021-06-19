#include <TransformManager.hpp>
using namespace UniEngine;
using namespace UniEngine;
void TransformManager::Init()
{
    GetInstance().m_transformQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(GetInstance().m_transformQuery, Transform(), GlobalTransform());
}

void TransformManager::LateUpdate()
{
    EntityManager::ForEach<Transform, GlobalTransform>(
        JobManager::PrimaryWorkers(),
        GetInstance().m_transformQuery,
        [](int i, Entity entity, Transform &ltp, GlobalTransform &ltw) {
            if (EntityManager::IsEntityStatic(entity) || !EntityManager::GetParent(entity).IsNull())
                return;
            ltw.m_value = ltp.m_value;
            CalculateLtwRecursive(ltw, entity);
        },
        false);
}

void TransformManager::CalculateLtwRecursive(const GlobalTransform &pltw, Entity entity)
{
    if (EntityManager::IsEntityStatic(entity))
        return;
    for (const auto &i : EntityManager::GetChildren(entity))
    {
        auto ltp = EntityManager::GetComponentData<Transform>(i);
        GlobalTransform ltw;
        ltw.m_value = pltw.m_value * ltp.m_value;
        EntityManager::SetComponentData<GlobalTransform>(i, ltw);
        CalculateLtwRecursive(ltw, i);
    }
}
