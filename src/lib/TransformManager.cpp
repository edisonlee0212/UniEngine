#include <ProfilerManager.hpp>
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
    CalculateTransformGraphs(EntityManager::GetCurrentScene());
}

void TransformManager::CalculateTransformGraph(const std::shared_ptr<Scene>& scene,
                                               std::vector<EntityMetadata> &entityInfos, const GlobalTransform &pltw, Entity parent)
{
    auto &transformManager = GetInstance();
    EntityMetadata &entityInfo = entityInfos.at(parent.GetIndex());
    for (const auto &entity : entityInfo.m_children)
    {
        auto *transformStatus = reinterpret_cast<GlobalTransformUpdateFlag *>(
            EntityManager::GetDataComponentPointer(scene, entity.GetIndex(), typeid(GlobalTransformUpdateFlag).hash_code()));
        GlobalTransform ltw;
        if (transformStatus->m_value)
        {
            ltw = entity.GetDataComponent<GlobalTransform>();
            reinterpret_cast<Transform *>(EntityManager::GetDataComponentPointer(scene, entity.GetIndex(), typeid(Transform).hash_code()))
                ->m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            transformStatus->m_value = false;
        }
        else
        {
            auto ltp = EntityManager::GetDataComponent<Transform>(scene, entity.GetIndex());
            ltw.m_value = pltw.m_value * ltp.m_value;
            *reinterpret_cast<GlobalTransform *>(
                EntityManager::GetDataComponentPointer(scene, entity.GetIndex(), typeid(GlobalTransform).hash_code())) = ltw;
        }
        CalculateTransformGraph(scene, entityInfos, ltw, entity);
    }
}
void TransformManager::CalculateTransformGraphs(const std::shared_ptr<Scene>& scene)
{
    if (!scene)
        return;
    auto& entityInfos = scene->m_sceneDataStorage.m_entityInfos;
    ProfilerManager::StartEvent("TransformManager");
    auto &transformManager = GetInstance();
    EntityManager::ForEach<Transform, GlobalTransform, GlobalTransformUpdateFlag>(scene,
        JobManager::PrimaryWorkers(),
        transformManager.m_transformQuery,
        [&](int i,
            Entity entity,
            Transform &transform,
            GlobalTransform &globalTransform,
            GlobalTransformUpdateFlag &transformStatus) {
            EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.GetIndex());
            if (!entityInfo.m_parent.IsNull())
                return;
            if (transformStatus.m_value)
            {
                transform.m_value = globalTransform.m_value;
            }
            else
                globalTransform.m_value = transform.m_value;
            transformStatus.m_value = false;
            CalculateTransformGraph(scene, entityInfos, globalTransform, entity);
        },
        false);
    transformManager.m_physicsSystemOverride = false;
    ProfilerManager::EndEvent("TransformManager");
}
void TransformManager::CalculateTransformGraphForDescendents(const Entity &entity)
{
    auto scene = entity.GetOwner();
    if(!scene) return;
    auto& entityInfos = scene->m_sceneDataStorage.m_entityInfos;
    CalculateTransformGraph(scene, entityInfos, EntityManager::GetDataComponent<GlobalTransform>(scene, entity.GetIndex()), entity);
}
