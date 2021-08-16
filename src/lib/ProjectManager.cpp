#include "ProjectManager.hpp"
#include <AssetManager.hpp>
#include <EntityManager.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void Project::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_assetRegistryPath" << YAML::Value << m_assetRegistryPath.string();
    out << YAML::Key << "m_startScenePath" << YAML::Value << m_startScenePath.string();
}

void Project::Deserialize(const YAML::Node &in)
{
    m_assetRegistryPath = in["m_assetRegistryPath"].as<std::string>();
    m_startScenePath = in["m_startScenePath"].as<std::string>();
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

    if (std::filesystem::exists(path))
    {
        std::ifstream stream(path.string());
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        YAML::Node in = YAML::Load(stringStream.str());
        projectManager.m_currentProject->Deserialize(in);
        LoadAssetRegistry();
        auto scene = AssetManager::Import<Scene>(projectManager.m_currentProject->m_startScenePath);
        EntityManager::Attach(scene);
        UNIENGINE_LOG("Found and loaded project");
    }
    else
    {
        auto directory = projectManager.m_projectPath;
        directory.remove_filename();
        projectManager.m_currentProject->m_assetRegistryPath = directory / ".ueassetregistry";
        projectManager.m_currentProject->m_startScenePath = directory / "New Scene.uescene";
        auto scene = AssetManager::CreateAsset<Scene>("New Scene");
        scene->SetPath(projectManager.m_currentProject->m_startScenePath);
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
    }
    AssetManager::GetAssetFolderPath();
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
    if (!currentScene->m_saved)
    {
        currentScene->Save();
    }
    projectManager.m_currentProject->m_startScenePath = currentScene->GetPath();
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
void ProjectManager::RefreshAssetRegistry()
{
}
void ProjectManager::OnGui()
{
    auto &projectManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            FileUtils::SaveFile("Create...##ProjectManager", ".ueproj", [](const std::string &filePath) {
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
            FileUtils::OpenFile("Load...##ProjectManager", ".ueproj", [&](const std::string &filePath) {
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
                RefreshAssetRegistry();
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
        resourceFolder / "Projects" / std::to_string(Handle().GetValue()) / "New Project.ueproj";
    CreateOrLoadProject(path);
}
