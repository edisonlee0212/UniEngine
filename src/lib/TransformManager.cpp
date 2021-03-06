#include <Articulation.hpp>
#include <RigidBody.hpp>
#include <TransformManager.hpp>
#include <ProfilerManager.hpp>
using namespace UniEngine;
using namespace UniEngine;
void TransformManager::Init()
{
    GetInstance().m_transformQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(GetInstance().m_transformQuery, Transform(), GlobalTransform());
}

void TransformManager::PreUpdate()
{
    ProfilerManager::StartEvent("TransformManager");
    auto &transformManager = GetInstance();
    EntityManager::ForEach<Transform, GlobalTransform, GlobalTransformUpdateFlag>(
        JobManager::PrimaryWorkers(),
        transformManager.m_transformQuery,
        [&](int i,
            Entity entity,
            Transform &transform,
            GlobalTransform &globalTransform,
            GlobalTransformUpdateFlag &transformStatus) {
            EntityInfo& entityInfo = EntityManager::GetInstance().m_entityInfos->at(entity.GetIndex());
            if ((!transformManager.m_physicsSystemOverride && entityInfo.m_static) ||
                !entityInfo.m_parent.IsNull())
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
    ProfilerManager::EndEvent("TransformManager");
}

void TransformManager::CalculateLtwRecursive(const GlobalTransform &pltw, Entity parent)
{
    auto &transformManager = GetInstance();
    EntityInfo& entityInfo = EntityManager::GetInstance().m_entityInfos->at(parent.GetIndex());
    if (!transformManager.m_physicsSystemOverride && entityInfo.m_static)
        return;
    for (const auto &entity : entityInfo.m_children)
    {
        auto* transformStatus = reinterpret_cast<GlobalTransformUpdateFlag *>(
                                   EntityManager::GetDataComponentPointer(entity, typeid(GlobalTransformUpdateFlag).hash_code()));
        GlobalTransform ltw;
        if (transformStatus->m_value)
        {
            ltw = entity.GetDataComponent<GlobalTransform>();
            reinterpret_cast<Transform *>(EntityManager::GetDataComponentPointer(entity, typeid(Transform).hash_code()))->m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            transformStatus->m_value = false;
        }
        else
        {
            auto ltp = EntityManager::GetDataComponent<Transform>(entity);
            ltw.m_value = pltw.m_value * ltp.m_value;
            *reinterpret_cast<GlobalTransform *>(EntityManager::GetDataComponentPointer(entity, typeid(GlobalTransform).hash_code())) = ltw;
        }
        CalculateLtwRecursive(ltw, entity);
    }
}
