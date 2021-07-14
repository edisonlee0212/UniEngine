#include <Articulation.hpp>
#include <RigidBody.hpp>
#include <TransformManager.hpp>
using namespace UniEngine;
using namespace UniEngine;
void TransformManager::Init()
{
    GetInstance().m_transformQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(GetInstance().m_transformQuery, Transform(), GlobalTransform());
}

void TransformManager::PreUpdate()
{
    auto& transformManager = GetInstance();
    EntityManager::ForEach<Transform, GlobalTransform>(
        JobManager::PrimaryWorkers(),
        transformManager.m_transformQuery,
        [&](int i, Entity entity, Transform &transform, GlobalTransform &globalTransform) {
            if ((!transformManager.m_physicsSystemOverride && EntityManager::IsEntityStatic(entity)) ||
                !EntityManager::GetParent(entity).IsNull())
                return;
            if (transformManager.m_physicsSystemOverride &&
                (entity.HasPrivateComponent<RigidBody>() || entity.HasPrivateComponent<Articulation>()))
            {
                transform.m_value = globalTransform.m_value;
            }
            else
            {
                globalTransform.m_value = transform.m_value;
            }
            CalculateLtwRecursive(globalTransform, entity);
        },
        false);
    transformManager.m_physicsSystemOverride = false;
}

void TransformManager::CalculateLtwRecursive(const GlobalTransform &pltw, Entity parent)
{
    if (!GetInstance().m_physicsSystemOverride && EntityManager::IsEntityStatic(parent))
        return;
    for (const auto &entity : EntityManager::GetChildren(parent))
    {
        GlobalTransform ltw;
        if (GetInstance().m_physicsSystemOverride &&
            (entity.HasPrivateComponent<RigidBody>() || entity.HasPrivateComponent<Articulation>()))
        {
            ltw = entity.GetComponentData<GlobalTransform>();
            Transform ltp;
            ltp.m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            entity.SetComponentData(ltp);
        }else
        {
            auto ltp = EntityManager::GetComponentData<Transform>(entity);
            ltw.m_value = pltw.m_value * ltp.m_value;
            entity.SetComponentData(ltw);
        }
        CalculateLtwRecursive(ltw, entity);
    }
}
