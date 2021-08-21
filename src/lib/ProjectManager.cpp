#include "ProjectManager.hpp"
#include <AssetManager.hpp>
#include <EntityManager.hpp>
#include <RenderManager.hpp>
#include <Application.hpp>
#include <PhysicsManager.hpp>
using namespace UniEngine;

void Project::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_assetRegistryPath" << YAML::Value << m_assetRegistryPath.string();
    out << YAML::Key << "m_startScene" << YAML::Value << m_startScene.GetAssetHandle().GetValue();
}

void Project::Deserialize(const YAML::Node &in)
{
    m_assetRegistryPath = in["m_assetRegistryPath"].as<std::string>();
    m_startScene.m_assetHandle = Handle(in["m_startScene"].as<uint64_t>());
}

void ProjectManager::CreateOrLoadProject(const std::filesystem::path &path)
{
    auto &projectManager = GetInstance();
    if (path == projectManager.m_projectPath)
        return;
    projectManager.m_projectPath = path;
    projectManager.m_currentProjectName = path.stem().string();
    projectManager.m_currentProject = SerializationManager::ProduceSerializable<Project>();
    projectManager.m_assetRegistry = SerializationManager::ProduceSerializable<AssetRegistry>();
    Application::Reset();
    std::shared_ptr<Scene> scene;
    if (std::filesystem::exists(path))
    {
        std::ifstream stream(path.string());
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        YAML::Node in = YAML::Load(stringStream.str());
        projectManager.m_currentProject->Deserialize(in);
        LoadAssetRegistry();
        auto sceneHandle = projectManager.m_currentProject->m_startScene.GetAssetHandle();
        auto sceneRecord = projectManager.m_assetRegistry->m_assetRecords[sceneHandle];
        scene = AssetManager::CreateAsset<Scene>(sceneHandle, sceneRecord.m_name);
        scene->SetPath(sceneRecord.m_filePath);
        EntityManager::Attach(scene);
        scene->Load();
        EntityManager::Attach(scene);
        UNIENGINE_LOG("Found and loaded project");
    }
    else
    {
        auto directory = projectManager.m_projectPath;
        directory.remove_filename();
        projectManager.m_currentProject->m_assetRegistryPath = directory / ".ueassetregistry";
        Application::Reset();
        scene = AssetManager::CreateAsset<Scene>("New Scene");
        scene->SetPath(directory / "New Scene.uescene");
        projectManager.m_currentProject->m_startScene = scene;
        EntityManager::Attach(scene);
#pragma region Main Camera
        const auto mainCameraEntity = EntityManager::CreateEntity("Main Camera");
        Transform cameraLtw;
        cameraLtw.SetPosition(glm::vec3(0.0f, 5.0f, 10.0f));
        cameraLtw.SetEulerRotation(glm::radians(glm::vec3(0, 0, 15)));
        mainCameraEntity.SetDataComponent(cameraLtw);
        auto mainCameraComponent = mainCameraEntity.GetOrSetPrivateComponent<Camera>().lock();
        RenderManager::SetMainCamera(mainCameraComponent);
        mainCameraComponent->m_skybox = DefaultResources::Environmental::DefaultSkybox;
#pragma endregion

        EntityManager::GetOrCreateSystem<PhysicsSystem>(SystemGroup::SimulationSystemGroup);
    }
    if(projectManager.m_newSceneCustomizer.has_value()) projectManager.m_newSceneCustomizer.value()();
}

std::filesystem::path ProjectManager::GetProjectPath()
{
    auto &path = GetInstance().m_projectPath;
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directories(path);
    }
    return path;
}
void ProjectManager::SaveProject()
{
    auto &projectManager = GetInstance();
    auto currentScene = EntityManager::GetCurrentScene();
    currentScene->Save();

    projectManager.m_currentProject->m_startScene = currentScene;
    projectManager.m_projectPath.replace_filename(projectManager.m_currentProjectName).replace_extension(".ueproj");
    auto directory = projectManager.m_projectPath;
    directory.remove_filename();
    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }
    YAML::Emitter out;
    out << YAML::BeginMap;
    projectManager.m_currentProject->Serialize(out);
    out << YAML::EndMap;
    std::ofstream fout(projectManager.m_projectPath.string());
    fout << out.c_str();
    fout.flush();
    SaveAssetRegistry();
}
void ProjectManager::SaveAssetRegistry()
{
    auto &projectManager = GetInstance();
    auto &path = projectManager.m_currentProject->m_assetRegistryPath;
    auto directory = path;
    directory.remove_filename();
    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }
    YAML::Emitter out;

    out << YAML::BeginMap;
    projectManager.m_assetRegistry->Serialize(out);
    out << YAML::EndMap;
    std::ofstream fout(path.string());
    fout << out.c_str();
    fout.flush();
}
void ProjectManager::LoadAssetRegistry()
{
    auto &projectManager = GetInstance();
    auto path = projectManager.m_currentProject->m_assetRegistryPath;
    projectManager.m_assetRegistry = SerializationManager::ProduceSerializable<AssetRegistry>();
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    projectManager.m_assetRegistry->Deserialize(in);
}
void ProjectManager::ScanProjectFolder()
{

}

void ProjectManager::OnGui()
{
    auto &projectManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            ImGui::Text(("Current Project path: " + projectManager.m_projectPath.string()).c_str());

            FileUtils::SaveFile("New...##ProjectManager", ".ueproj", [](const std::string &filePath) {
                std::filesystem::path path = filePath;
                path.replace_extension(".ueproj");
                try
                {
                    CreateOrLoadProject(path);
                    UNIENGINE_LOG("Saved to " + path.string());
                }
                catch (std::exception &e)
                {
                    UNIENGINE_ERROR("Failed to save to " + path.string());
                }
            });
            FileUtils::OpenFile("Open...##ProjectManager", ".ueproj", [&](const std::string &filePath) {
                std::filesystem::path path = filePath;
                path.replace_extension(".ueproj");
                try
                {
                    CreateOrLoadProject(path);
                    UNIENGINE_LOG("Loaded from " + filePath);
                }
                catch (std::exception &e)
                {
                    UNIENGINE_ERROR("Failed to load from " + filePath);
                }
            });
            if (ImGui::Button("Save"))
            {
                SaveProject();
            }
            if (ImGui::Button("Refresh Assets"))
            {
                ScanProjectFolder();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void ProjectManager::Init()
{
    auto &projectManager = GetInstance();
    std::filesystem::path resourceFolder = UNIENGINE_RESOURCE_FOLDER;
    std::filesystem::path path =
        resourceFolder / "Temp Projects" / std::to_string(Handle().GetValue()) / "New Project.ueproj";
    CreateOrLoadProject(path);
}
void ProjectManager::SetScenePostLoadActions(const std::function<void()> &actions)
{
    GetInstance().m_newSceneCustomizer = actions;
}
