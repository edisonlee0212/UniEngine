#include <ProfilerLayer.hpp>
#include <RigidBody.hpp>
#include <TransformLayer.hpp>
using namespace UniEngine;
using namespace UniEngine;
void TransformLayer::OnCreate()
{
    m_transformQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(m_transformQuery, Transform(), GlobalTransform());
}

void TransformLayer::PreUpdate()
{
    CalculateTransformGraphs(EntityManager::GetCurrentScene());
}

void TransformLayer::CalculateTransformGraph(
    const std::shared_ptr<Scene> &scene,
    std::vector<EntityMetadata> &entityInfos,
    const GlobalTransform &pltw,
    Entity parent)
{
    EntityMetadata &entityInfo = entityInfos.at(parent.GetIndex());
    for (const auto &entity : entityInfo.m_children)
    {
        auto *transformStatus = reinterpret_cast<GlobalTransformUpdateFlag *>(EntityManager::GetDataComponentPointer(
            scene, entity.GetIndex(), typeid(GlobalTransformUpdateFlag).hash_code()));
        GlobalTransform ltw;
        if (transformStatus->m_value)
        {
            ltw = entity.GetDataComponent<GlobalTransform>();
            reinterpret_cast<Transform *>(
                EntityManager::GetDataComponentPointer(scene, entity.GetIndex(), typeid(Transform).hash_code()))
                ->m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            transformStatus->m_value = false;
        }
        else
        {
            auto ltp = EntityManager::GetDataComponent<Transform>(scene, entity.GetIndex());
            ltw.m_value = pltw.m_value * ltp.m_value;
            *reinterpret_cast<GlobalTransform *>(EntityManager::GetDataComponentPointer(
                scene, entity.GetIndex(), typeid(GlobalTransform).hash_code())) = ltw;
        }
        CalculateTransformGraph(scene, entityInfos, ltw, entity);
    }
}
void TransformLayer::CalculateTransformGraphs(const std::shared_ptr<Scene> &scene, bool checkStatic)
{
    if (!scene)
        return;
    auto &entityInfos = scene->m_sceneDataStorage.m_entityInfos;
    ProfilerLayer::StartEvent("TransformManager");
    EntityManager::ForEach<Transform, GlobalTransform, GlobalTransformUpdateFlag>(
        scene,
        JobManager::Workers(),
        m_transformQuery,
        [&](int i,
            Entity entity,
            Transform &transform,
            GlobalTransform &globalTransform,
            GlobalTransformUpdateFlag &transformStatus) {
            EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.GetIndex());
            if (!entityInfo.m_parent.IsNull())
                return;
            if (checkStatic && entityInfo.m_static)
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
    m_physicsSystemOverride = false;
    ProfilerLayer::EndEvent("TransformManager");
}
void TransformLayer::CalculateTransformGraphForDescendents(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    if (!scene)
        return;
    auto &entityInfos = scene->m_sceneDataStorage.m_entityInfos;
    CalculateTransformGraph(
        scene, entityInfos, EntityManager::GetDataComponent<GlobalTransform>(scene, entity.GetIndex()), entity);
}
