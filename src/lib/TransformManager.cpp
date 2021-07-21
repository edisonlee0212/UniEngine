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
    EntityManager::ForEach<Transform, GlobalTransform, GlobalTransformUpdateFlag>(
        JobManager::PrimaryWorkers(),
        transformManager.m_transformQuery,
        [&](int i,
            Entity entity,
            Transform &transform,
            GlobalTransform &globalTransform,
            GlobalTransformUpdateFlag &transformStatus) {
            if ((!transformManager.m_physicsSystemOverride && EntityManager::IsEntityStatic(entity)) ||
                !EntityManager::GetParent(entity).IsNull())
                return;
            if (transformStatus.m_value)
            {
                transform.m_value = globalTransform.m_value;
            }
            else
                globalTransform.m_value = transform.m_value;
            transformStatus.m_value = false;
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
        auto transformStatus = entity.GetDataComponent<GlobalTransformUpdateFlag>();
        GlobalTransform ltw;
        if (transformStatus.m_value)
        {
            ltw = entity.GetDataComponent<GlobalTransform>();
            Transform ltp;
            ltp.m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            *reinterpret_cast<Transform *>(EntityManager::GetDataComponentPointer(entity, typeid(Transform).hash_code())) = ltp;
        }
        else
        {
            auto ltp = EntityManager::GetDataComponent<Transform>(entity);
            ltw.m_value = pltw.m_value * ltp.m_value;
            *reinterpret_cast<GlobalTransform *>(EntityManager::GetDataComponentPointer(entity, typeid(GlobalTransform).hash_code())) = ltw;
        }
        reinterpret_cast<GlobalTransformUpdateFlag *>(
            EntityManager::GetDataComponentPointer(entity, typeid(GlobalTransformUpdateFlag).hash_code()))->m_value = false;
        CalculateLtwRecursive(ltw, entity);
    }
}
