#include "ProfilerLayer.hpp"
#include "RigidBody.hpp"
#include "TransformLayer.hpp"
#include "Application.hpp"
#include "Scene.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;

DataComponentRegistration<Transform> TransformRegistry("Transform");
DataComponentRegistration<GlobalTransform> GlobalTransformRegistry("GlobalTransform");
DataComponentRegistration<GlobalTransformUpdateFlag> GlobalTransformUpdateFlagRegistry("GlobalTransformUpdateFlag");


void TransformLayer::OnCreate()
{
    m_transformQuery = Entities::CreateEntityQuery();
    Entities::SetEntityQueryAllFilters(m_transformQuery, Transform(), GlobalTransform());
}

void TransformLayer::PreUpdate()
{
    CalculateTransformGraphs(GetScene());
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
        auto *transformStatus = reinterpret_cast<GlobalTransformUpdateFlag *>(
            scene->GetDataComponentPointer(entity.GetIndex(), typeid(GlobalTransformUpdateFlag).hash_code()));
        GlobalTransform ltw;
        if (transformStatus->m_value)
        {
            ltw = scene->GetDataComponent<GlobalTransform>(entity.GetIndex());
            reinterpret_cast<Transform *>(
                scene->GetDataComponentPointer(entity.GetIndex(), typeid(Transform).hash_code()))
                ->m_value = glm::inverse(pltw.m_value) * ltw.m_value;
            transformStatus->m_value = false;
        }
        else
        {
            auto ltp = scene->GetDataComponent<Transform>(entity.GetIndex());
            ltw.m_value = pltw.m_value * ltp.m_value;
            *reinterpret_cast<GlobalTransform *>(
                scene->GetDataComponentPointer(entity.GetIndex(), typeid(GlobalTransform).hash_code())) = ltw;
        }
        CalculateTransformGraph(scene, entityInfos, ltw, entity);
    }
}
void TransformLayer::CalculateTransformGraphs(const std::shared_ptr<Scene> &scene, bool checkStatic)
{
    if (!scene)
        return;
    auto &entityInfos = scene->m_sceneDataStorage.m_entityMetadataList;
    ProfilerLayer::StartEvent("TransformManager");
    scene->ForEach<Transform, GlobalTransform, GlobalTransformUpdateFlag>(
        Jobs::Workers(),
        m_transformQuery,
        [&](int i,
            Entity entity,
            Transform &transform,
            GlobalTransform &globalTransform,
            GlobalTransformUpdateFlag &transformStatus) {
            EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityMetadataList.at(entity.GetIndex());
            if (entityInfo.m_parent.GetIndex() != 0)
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
    auto &entityInfos = scene->m_sceneDataStorage.m_entityMetadataList;
    CalculateTransformGraph(
        scene, entityInfos, scene->GetDataComponent<GlobalTransform>(entity.GetIndex()), entity);
}
