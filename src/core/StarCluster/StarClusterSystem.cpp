#include "ProjectManager.hpp"
#include "Editor.hpp"
#include "Serialization.hpp"
#include "StarCluster/StarClusterSystem.hpp"
#include "Graphics.hpp"
using namespace Galaxy;

void StarClusterPattern::OnInspect()
{
    static bool autoApply = true;
    ImGui::Checkbox("Auto apply", &autoApply);
    if (!autoApply && ImGui::Button("Apply"))
        Apply();
    bool needUpdate = false;
    float ySpread = m_ySpread;
    float xzSpread = m_xzSpread;
    float diskDiameter = m_diskDiameter;
    float diskEccentricity = m_diskEccentricity;
    float coreProportion = m_coreProportion;
    float coreEccentricity = m_coreEccentricity;
    float centerDiameter = m_centerDiameter;
    float centerEccentricity = m_centerEccentricity;
    float diskSpeed = m_diskSpeed;
    float coreSpeed = m_coreSpeed;
    float centerSpeed = m_centerSpeed;
    float diskTiltX = m_diskTiltX;
    float diskTiltZ = m_diskTiltZ;
    float coreTiltX = m_coreTiltX;
    float coreTiltZ = m_coreTiltZ;
    float centerTiltX = m_centerTiltX;
    float centerTiltZ = m_centerTiltZ;
    float twist = m_twist;
    glm::vec3 centerOffset = m_centerOffset;
    glm::vec3 centerPosition = m_centerPosition;
    if (ImGui::TreeNode("Shape"))
    {
        if (ImGui::DragFloat("Y Spread", &ySpread, 0.001f, 0.0f, 1.0f, "%.3f"))
        {
            m_ySpread = ySpread;
            needUpdate = true;
        }
        if (ImGui::DragFloat("XZ Spread", &xzSpread, 0.001f, 0.0f, 1.0f, "%.3f"))
        {
            m_xzSpread = xzSpread;
            needUpdate = true;
        }

        if (ImGui::DragFloat("Disk size", &diskDiameter, 1.0f, 1.0f, 10000.0f))
        {
            m_diskDiameter = diskDiameter;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Disk eccentricity", &diskEccentricity, 0.01f, 0.0f, 1.0f))
        {
            m_diskEccentricity = diskEccentricity;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Core proportion", &coreProportion, 0.01f, 0.0f, 1.0f))
        {
            m_coreProportion = coreProportion;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Core eccentricity", &coreEccentricity, 0.01f, 0, 1))
        {
            m_coreEccentricity = coreEccentricity;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Center size", &centerDiameter, 1.0f, 0, 9999))
        {
            m_centerDiameter = centerDiameter;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Center eccentricity", &centerEccentricity, 0.01f, 0, 1))
        {
            m_centerEccentricity = centerEccentricity;
            needUpdate = true;
        }
        ImGui::TreePop();
        if (ImGui::DragFloat3("Center offset", &centerOffset.x, 1.0f, -10000.0f, 10000.0f))
        {
            m_centerOffset = centerOffset;
            needUpdate = true;
        }
        if (ImGui::DragFloat3("Center position", &centerPosition.x, 1.0f, -10000.0f, 10000.0f))
        {
            m_centerPosition = centerPosition;
            needUpdate = true;
        }
    }
    if (ImGui::TreeNode("Movement"))
    {
        if (ImGui::DragFloat("Disk speed", &diskSpeed, 0.1f, -100, 100))
        {
            m_diskSpeed = diskSpeed;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Core speed", &coreSpeed, 0.1f, -100, 100))
        {
            m_coreSpeed = coreSpeed;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Center speed", &centerSpeed, 0.1f, -100, 100))
        {
            m_centerSpeed = centerSpeed;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Disk X tilt", &diskTiltX, 1.0f, -180.0f, 180.0f))
        {
            m_diskTiltX = diskTiltX;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Disk Z tilt", &diskTiltZ, 1.0f, -180.0f, 180.0f))
        {
            m_diskTiltZ = diskTiltZ;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Core X tilt", &coreTiltX, 1.0f, -180.0f, 180.0f))
        {
            m_coreTiltX = coreTiltX;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Core Z tilt", &coreTiltZ, 1.0f, -180.0f, 180.0f))
        {
            m_coreTiltZ = coreTiltZ;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Center X tilt", &centerTiltX, 1.0f, -180.0f, 180.0f))
        {
            m_centerTiltX = centerTiltX;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Center Z tilt", &centerTiltZ, 1.0f, -180.0f, 180.0f))
        {
            m_centerTiltZ = centerTiltZ;
            needUpdate = true;
        }
        if (ImGui::DragFloat("Twist", &twist, 1.0f, -720.0f, 720.0f))
        {
            m_twist = twist;
            needUpdate = true;
        }
        ImGui::TreePop();
    }
    bool colorUpdate = false;
    if (ImGui::TreeNode("Rendering"))
    {
        if (ImGui::ColorEdit3("Disk Color", &m_diskColor.x, 0.1))
            colorUpdate = true;
        if (ImGui::DragFloat("Disk Color Intensity", &m_diskEmissionIntensity, 0.01f, 1.0f, 10.0f))
            colorUpdate = true;
        if (ImGui::ColorEdit3("Core Color", &m_coreColor.x, 0.1))
            colorUpdate = true;
        if (ImGui::DragFloat("Core Color Intensity", &m_coreEmissionIntensity, 0.01f, 1.0f, 10.0f))
            colorUpdate = true;
        if (ImGui::ColorEdit3("Center Color", &m_centerColor.x, 0.1))
            colorUpdate = true;
        if (ImGui::DragFloat("Center Color Intensity", &m_centerEmissionIntensity, 0.01f, 1.0f, 10.0f))
            colorUpdate = true;
        ImGui::TreePop();
    }

    if (needUpdate)
    {
        Apply(true);
    }
    else if (colorUpdate)
    {
        Apply(true, true);
    }
}

void StarClusterPattern::Apply(const bool &forceUpdateAllStars, const bool &onlyUpdateColors)
{
    SetAb();
    Application::GetActiveScene()->ForEach<StarInfo, StarClusterIndex, StarOrbit, StarOrbitOffset, StarOrbitProportion, SurfaceColor>(
        Jobs::Workers(),
        [&](int i,
            Entity entity,
            StarInfo &starInfo,
            StarClusterIndex &starClusterIndex,
            StarOrbit &starOrbit,
            StarOrbitOffset &starOrbitOffset,
            StarOrbitProportion &starOrbitProportion,
            SurfaceColor &surfaceColor) {
            if (!forceUpdateAllStars && starInfo.m_initialized)
                return;
            if (starClusterIndex.m_value != this->m_starClusterIndex.m_value)
                return;
            starInfo.m_initialized = true;
            const auto proportion = starOrbitProportion.m_value;
            if (!onlyUpdateColors)
            {
                starOrbitOffset = GetOrbitOffset(proportion);
                starOrbit = GetOrbit(proportion);
            }
            surfaceColor.m_value = GetColor(proportion);
            surfaceColor.m_intensity = GetIntensity(proportion);
        });
}

void StarClusterSystem::OnInspect()
{
    ImGui::InputFloat("Time", &m_galaxyTime);
    static int amount = 10000;
    ImGui::DragInt("Amount", &amount, 1, 1, 100000);
    if (amount < 1)
        amount = 1;
    if (ImGui::CollapsingHeader("Star clusters", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int i = 0;
        for (auto &pattern : m_starClusterPatterns)
        {
            i++;
            if (ImGui::TreeNodeEx((std::to_string(i) + ": " + pattern.m_name).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::TreeNodeEx("Properties", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    pattern.OnInspect();
                    ImGui::TreePop();
                }
                if (ImGui::Button(("Add " + std::to_string(amount) + " stars").c_str()))
                {
                    PushStars(pattern, amount);
                }
                ImGui::TreePop();
            }
        }
    }
    if (ImGui::CollapsingHeader("Star removal", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button(("Remove " + std::to_string(amount) + " stars").c_str()))
            RandomlyRemoveStars(amount);
        if (ImGui::Button("Remove all stars"))
            ClearAllStars();
    }
    if (ImGui::CollapsingHeader("Start time control", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Speed", &m_speed, 1.0f, 0.0f, 40000.0f);
        ImGui::DragFloat("Star Size", &m_size, 0.01f, 0.01f, 10.0f);
    }
    ImGui::Text("Status:");
    ImGui::InputFloat("Apply time", &m_applyPositionTimer, 0, 0, "%.5f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Copy time", &m_copyPositionTimer, 0, 0, "%.5f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Calculation time", &m_calcPositionResult, 0, 0, "%.5f", ImGuiInputTextFlags_ReadOnly);
}

void StarClusterSystem::CalculateStarPositionSync()
{
    auto scene = GetScene();
    m_calcPositionTimer = Application::Time().CurrentTime();
    // StarOrbitProportion: The relative position of the star, here it is used to calculate the speed of the star
    // around its orbit. StarPosition: The final output of this operation, records the position of the star in the
    // galaxy. StarOrbit: The orbit which contains the function for calculating the position based on current time
    // and proportion value. StarOrbitOffset: The position offset of the star, used to add irregularity to the
    // position.
    scene->ForEach<StarOrbitProportion, StarPosition, StarOrbit, StarOrbitOffset>(
        Jobs::Workers(),
        m_starQuery,
        [=](int i,
            Entity entity,
            StarOrbitProportion &starProportion,
            StarPosition &starPosition,
            StarOrbit &starOrbit,
            StarOrbitOffset &starOrbitOffset) {
            // Code here will be exec in parallel
            starPosition.m_value =
                starOrbit.GetPoint(starOrbitOffset.m_value, starProportion.m_value * 360.0f + m_galaxyTime, true);
        },
        false);
    const auto usedTime = Application::Time().CurrentTime() - m_calcPositionTimer;
    m_calcPositionResult = m_calcPositionResult * m_counter / (m_counter + 1) + usedTime / (m_counter + 1);

    // Copy data for rendering.
    m_useFront = true;
    ApplyPosition();
    CopyPosition();
}

void StarClusterSystem::ApplyPosition()
{
    auto scene = GetScene();
    m_applyPositionTimer = Application::Time().CurrentTime();
    scene->ForEach<StarPosition, GlobalTransform, Transform, SurfaceColor, DisplayColor>(
        Jobs::Workers(),
        m_starQuery,
        [this](
            int i,
            Entity entity,
            StarPosition &position,
            GlobalTransform &globalTransform,
            Transform &transform,
            SurfaceColor &surfaceColor,
            DisplayColor &displayColor) {
            // Code here will be exec in parallel
            globalTransform.m_value =
                glm::translate(glm::vec3(position.m_value) / 20.0f) * glm::scale(m_size * glm::vec3(1.0f));
            transform.m_value = globalTransform.m_value;
            displayColor.m_value = surfaceColor.m_value;
            displayColor.m_intensity = surfaceColor.m_intensity;
        },
        false);
    m_applyPositionTimer = Application::Time().CurrentTime() - m_applyPositionTimer;
}

void StarClusterSystem::CopyPosition(const bool &reverse)
{
    auto scene = GetScene();
    bool check = reverse ? !m_useFront : m_useFront;
    auto matrices = check ? scene->GetOrSetPrivateComponent<Particles>(m_rendererFront.Get()).lock()->m_matrices
                          : scene->GetOrSetPrivateComponent<Particles>(m_rendererBack.Get()).lock()->m_matrices;
    const auto starAmount = scene->GetEntityAmount(m_starQuery);
    matrices->RefMatrices().resize(starAmount);
    matrices->RefColors().resize(starAmount);
    scene->ForEach<GlobalTransform, DisplayColor>(
        Jobs::Workers(),
        m_starQuery,
        [&](int i, Entity entity, GlobalTransform &globalTransform, DisplayColor &displayColor) {
            matrices->RefMatrices()[i] = globalTransform.m_value;
			matrices->RefColors()[i] = glm::vec4(displayColor.m_value * displayColor.m_intensity, 1.0f);
        },
        false);
    matrices->Update();
}

void StarClusterSystem::OnCreate()
{
    Enable();
}

void StarClusterSystem::Update()
{
    auto scene = GetScene();
    m_galaxyTime += Application::Time().DeltaTime() * m_speed;

    // This method calculate the position for each star. Remove this line if you use your own implementation.
    CalculateStarPositionSync();

    // Do not touch below functions.
    m_counter++;
    /*
    Gizmos::DrawGizmoMeshInstancedColored(
        DefaultResources::Primitives::Cube,
        m_useFront ? m_frontColors : m_backColors,
        m_useFront ? scene->GetOrSetPrivateComponent<Particles>(m_rendererFront.Get()).lock()->m_matrices->m_matrices
                   : scene->GetOrSetPrivateComponent<Particles>(m_rendererBack.Get()).lock()->m_matrices->m_matrices,
        glm::mat4(1.0f),
        1.0f);
        */
}

void StarClusterSystem::PushStars(StarClusterPattern &pattern, const size_t &amount)
{
    auto scene = GetScene();
    m_counter = 0;
    auto stars = scene->CreateEntities(m_starArchetype, amount, "Star");
    for (auto i = 0; i < amount; i++)
    {
        auto starEntity = stars[i];
        StarOrbitProportion proportion;
        proportion.m_value = glm::linearRand(0.0, 1.0);
        StarInfo starInfo;
        scene->SetDataComponent(starEntity, starInfo);
        scene->SetDataComponent(starEntity, proportion);
        scene->SetDataComponent(starEntity, pattern.m_starClusterIndex);
    }
    pattern.Apply();
}

void StarClusterSystem::RandomlyRemoveStars(const size_t &amount)
{
    m_counter = 0;
    std::vector<Entity> stars;

    auto scene = GetScene();

    scene->GetEntityArray(m_starQuery, stars);
    size_t residue = amount;
    for (const auto &i : stars)
    {
        if (residue > 0)
            residue--;
        else
            break;
        scene->DeleteEntity(i);
    }
}

void StarClusterSystem::ClearAllStars()
{
    m_counter = 0;
    std::vector<Entity> stars;
    auto scene = GetScene();
    scene->GetEntityArray(m_starQuery, stars);
    for (const auto &i : stars)
        scene->DeleteEntity(i);
}

void StarClusterSystem::FixedUpdate()
{
}

void StarClusterSystem::OnEnable()
{
    m_firstTime = true;
}
void StarClusterSystem::Start()
{
    auto scene = GetScene();
    if (m_rendererFront.Get().GetIndex() == 0 && m_rendererBack.Get().GetIndex() == 0)
    {
        m_rendererFront = scene->CreateEntity("Renderer 1");
        GlobalTransform ltw;
        ltw.SetScale(glm::vec3(1.0f));
        auto imr = scene->GetOrSetPrivateComponent<Particles>(m_rendererFront.Get()).lock();
        auto material = ProjectManager::CreateTemporaryAsset<Material>();
        imr->m_material.Set<Material>(material);
        imr->m_castShadow = false;
        imr->m_receiveShadow = false;
        material->m_materialProperties.m_emission = 3.0f;
        imr->m_mesh.Set<Mesh>(DefaultResources::Primitives::Cube);
        material->SetProgram(DefaultResources::GLPrograms::StandardInstancedColoredProgram);

        scene->SetDataComponent(m_rendererFront.Get(), ltw);

        m_rendererBack = scene->CreateEntity("Renderer 2");
        ltw.SetScale(glm::vec3(1.0f));
        auto imr2 = scene->GetOrSetPrivateComponent<Particles>(m_rendererBack.Get()).lock();
        imr2->m_material = imr->m_material;

        scene->SetDataComponent(m_rendererBack.Get(), ltw);
    }
    m_starClusterPatterns.resize(2);
    auto &starClusterPattern1 = m_starClusterPatterns[0];
    auto &starClusterPattern2 = m_starClusterPatterns[1];
    starClusterPattern1.m_starClusterIndex.m_value = 0;
    starClusterPattern2.m_starClusterIndex.m_value = 1;
    m_starQuery = Entities::CreateEntityQuery();
    m_starQuery.SetAllFilters(StarInfo());

    m_starArchetype = Entities::CreateEntityArchetype(
        "Star",
        GlobalTransform(),
        StarClusterIndex(),
        StarInfo(),
        StarOrbit(),
        StarOrbitOffset(),
        StarOrbitProportion(),
        StarPosition(),
        SelectionStatus(),
        OriginalColor(),
        SurfaceColor(),
        DisplayColor());
    m_firstTime = true;
}
void StarClusterSystem::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_speed" << YAML::Value << m_speed;
    out << YAML::Key << "m_size" << YAML::Value << m_size;
    out << YAML::Key << "m_galaxyTime" << YAML::Value << m_galaxyTime;

    m_rendererFront.Save("m_rendererFront", out);
    m_rendererBack.Save("m_rendererBack", out);
}
void StarClusterSystem::Deserialize(const YAML::Node &in)
{
    m_speed = in["m_speed"].as<float>();
    m_size = in["m_size"].as<float>();
    m_galaxyTime = in["m_galaxyTime"].as<float>();

    m_rendererFront.Load("m_rendererFront", in);
    m_rendererBack.Load("m_rendererBack", in);
}