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
    auto &transformManager = GetInstance();
    EntityManager::ForEach<Transform, GlobalTransform>(
        JobManager::PrimaryWorkers(),
        transformManager.m_transformQuery,
        [&](int i, Entity entity, Transform &transform, GlobalTransform &globalTransform) {
            if ((!transformManager.m_physicsSystemOverride && EntityManager::IsEntityStatic(entity)) ||
                !EntityManager::GetParent(entity).IsNull())
                return;
            bool overwrite = transformManager.m_physicsSystemOverride;
            if (overwrite)
            {
                if (entity.HasPrivateComponent<RigidBody>())
                {
                    auto &rigidBody = entity.GetPrivateComponent<RigidBody>();
                    if (!rigidBody.m_currentRegistered || rigidBody.m_kinematic)
                        overwrite = false;
                }
                else if (entity.HasPrivateComponent<Articulation>())
                {
                    auto &articulation = entity.GetPrivateComponent<Articulation>();
                    if (!articulation.m_currentRegistered)
                        overwrite = false;
                }
                else
                    overwrite = false;
            }

            if (overwrite)
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
    auto &transformManager = GetInstance();
    if (!transformManager.m_physicsSystemOverride && EntityManager::IsEntityStatic(parent))
        return;
    for (const auto &entity : EntityManager::GetChildren(parent))
    {
        GlobalTransform ltw;
        bool overwrite = transformManager.m_physicsSystemOverride;
        if (overwrite)
        {
            if (entity.HasPrivateComponent<RigidBody>())
            {
                auto &rigidBody = entity.GetPrivateComponent<RigidBody>();
                if (!rigidBody.m_currentRegistered || rigidBody.m_kinematic)
                    overwrite = false;
            }
            else if (entity.HasPrivateComponent<Articulation>())
            {
                auto &articulation = entity.GetPrivateComponent<Articulation>();
                if (!articulation.m_currentRegistered)
                    overwrite = false;
            }
            else
                overwrite = false;
        }
        if (overwrite)
        {
            ltw = entity.GetDataComponent<GlobalTransform>();
            Transform ltp;
            ltp.m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            entity.SetDataComponent(ltp);
        }
        else
        {
            auto ltp = EntityManager::GetDataComponent<Transform>(entity);
            ltw.m_value = pltw.m_value * ltp.m_value;
            entity.SetDataComponent(ltw);
        }
        CalculateLtwRecursive(ltw, entity);
    }
}
