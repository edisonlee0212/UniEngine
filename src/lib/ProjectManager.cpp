#include "ProjectManager.hpp"
#include <Application.hpp>
#include <AssetManager.hpp>
#include <EntityManager.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void Project::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_startScenePath" << YAML::Value << m_startScenePath.string();
}

void Project::Deserialize(const YAML::Node &in)
{
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
    projectManager.m_assetRegistry.Clear();
    Application::Reset();
    std::shared_ptr<Scene> scene;


    projectManager.m_currentProject->m_projectFolder = projectManager.m_currentFocusedFolder =
        std::make_shared<Folder>();
    auto directory = projectManager.m_projectPath.parent_path();
    projectManager.m_currentFocusedFolder->m_name = directory.filename().string();
    ScanProjectFolder();

    if (std::filesystem::exists(path))
    {
        std::ifstream stream(path.string());
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        YAML::Node in = YAML::Load(stringStream.str());
        projectManager.m_currentProject->Deserialize(in);
        Handle sceneHandle;
        if(projectManager.m_assetRegistry.Find(projectManager.m_currentProject->m_startScenePath, sceneHandle)){
            scene = std::dynamic_pointer_cast<Scene>(AssetManager::Get(sceneHandle));
            EntityManager::Attach(scene);
        }else{
            GenerateNewDefaultScene();
        }
        UNIENGINE_LOG("Found and loaded project");
    }
    else
    {
        GenerateNewDefaultScene();
    }


    if (projectManager.m_newSceneCustomizer.has_value())
        projectManager.m_newSceneCustomizer.value()();
}

std::filesystem::path ProjectManager::GetProjectPath()
{
    auto &path = GetInstance().m_projectPath;
    if (!std::filesystem::exists(path.parent_path()))
    {
        std::filesystem::create_directories(path.parent_path());
    }
    return path;
}
void ProjectManager::SaveProject()
{
    auto &projectManager = GetInstance();
    auto currentScene = EntityManager::GetCurrentScene();
    if(currentScene->GetPath().empty()){
        GenerateNewPath(currentScene->m_name, ".uescene");
    }
    currentScene->Save();
    projectManager.m_currentProject->m_startScenePath = currentScene->GetPath();
    projectManager.m_projectPath.replace_filename(projectManager.m_currentProjectName).replace_extension(".ueproj");
    auto directory = projectManager.m_projectPath.parent_path();
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
    ScanProjectFolder();
}
void ProjectManager::ScanProjectFolder()
{
    auto &projectManager = GetInstance();
    auto projectPath = projectManager.m_projectPath;
    if(projectPath.empty()) return;
    auto directory = projectManager.m_projectPath.parent_path();
    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }

    std::shared_ptr<Folder> currentFolder = projectManager.m_currentProject->m_projectFolder;
    ScanFolderHelper(directory, currentFolder);
}
void ProjectManager::ScanFolderHelper(const std::filesystem::path &folderPath, std::shared_ptr<Folder> &folder)
{
    FolderMetadataUpdater(folderPath, folder);
    for (const auto &entry : std::filesystem::directory_iterator(folderPath))
    {
        if (std::filesystem::is_directory(entry.path()))
        {
            auto folderName = entry.path().filename().string();
            auto search = folder->m_children.find(folderName);
            std::shared_ptr<Folder> childFolder;
            if (search == folder->m_children.end())
            {
                auto newFolder = std::make_shared<Folder>();
                newFolder->m_folderMetadata = FolderMetadata();
                newFolder->m_relativePath = GetRelativePath(folderPath);
                newFolder->m_name = folderName;
                folder->m_children[entry.path().string()] = newFolder;
                newFolder->m_parent = folder;
                childFolder = newFolder;
            }
            else
            {
                childFolder = search->second;
            }
            ScanFolderHelper(entry.path(), childFolder);
        }
    }
    auto it = folder->m_children.begin();
    while (it != folder->m_children.end())
    {
        if (!std::filesystem::exists(it->first))
        {
            it = folder->m_children.erase(it);
        }
        else
            it++;
    }
}
void ProjectManager::OnInspect()
{
    auto &projectManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            ImGui::Text(("Current Project path: " + projectManager.m_projectPath.string()).c_str());

            FileUtils::SaveFile(
                "Create or load New Project##ProjectManager", "Project", {".ueproj"}, [](const std::filesystem::path &filePath) {
                    try
                    {
                        CreateOrLoadProject(filePath);
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to create/load from " + filePath.string());
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
void ProjectManager::Init(const std::filesystem::path &projectPath)
{
    auto &projectManager = GetInstance();
    if (projectPath.empty())
    {
        std::filesystem::path path =
            std::filesystem::path(".\\temp") / "Projects" / std::to_string(Handle().GetValue()) / "New Project.ueproj";
        CreateOrLoadProject(path);
    }
    else
    {
        CreateOrLoadProject(projectPath);
    }
}
void ProjectManager::SetScenePostLoadActions(const std::function<void()> &actions)
{
    GetInstance().m_newSceneCustomizer = actions;
}
void ProjectManager::FolderMetadataUpdater(const std::filesystem::path &folderPath, std::shared_ptr<Folder> &folder)
{
    auto &projectManager = GetInstance();
    auto &assetManager = AssetManager::GetInstance();
    // 1. Find metadata file
    std::filesystem::path fileMetadataPath;
    bool found = false;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath))
    {
        if (!std::filesystem::is_directory(entry.path()) && entry.path().filename().string() == ".uemetadata")
        {
            found = true;
            fileMetadataPath = entry;
        }
    }
    // 2. Create metadata file if not exist
    bool metadataUpdated = false;
    FolderMetadata &folderMetadata = folder->m_folderMetadata;
    if (found)
    {
        folderMetadata.Load(fileMetadataPath);
    }
    else
    {
        fileMetadataPath = folderPath / ".uemetadata";
        metadataUpdated = true;
    }
    //Remove deleted files.
    auto it = folderMetadata.m_fileMap.begin();
    while (it != folderMetadata.m_fileMap.end())
    {
        if (!std::filesystem::exists(projectManager.m_projectPath.parent_path() / it->first))
        {
            auto handle = it->second;
            auto search = assetManager.m_assets.find(handle);
            if(search != assetManager.m_assets.end()){
                auto asset = search->second.lock();
                if(asset){
                    asset->SetPath("");
                }
            }
            metadataUpdated = true;
            folderMetadata.m_fileRecords.erase(handle);
            it = folderMetadata.m_fileMap.erase(it);
        }
        else
            it++;
    }
    // 3. Load metadata file and scan the folder. Update metadata if needed.
    auto &assetRegistry = projectManager.m_assetRegistry;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath))
    {
        if (!std::filesystem::is_directory(entry.path()) && entry.path().filename().string() != ".uemetadata")
        {
            FileRecord fileRecord;
            fileRecord.m_name = entry.path().stem().string();
            fileRecord.m_relativeFilePath = GetRelativePath(entry.path());
            auto search = folderMetadata.m_fileMap.find(GetRelativePath(entry.path()).string());
            Handle fileHandle = Handle();
            //Update assetRegistry
            auto typeSearch = assetManager.m_typeNames.find(entry.path().extension().string());
            bool isAsset = false;
            if (typeSearch != assetManager.m_typeNames.end())
            {
                fileRecord.m_typeName = typeSearch->second;
                isAsset = true;
            }
            else
            {
                fileRecord.m_typeName = "Binary";
            }
            assetRegistry.Find(fileRecord.m_relativeFilePath, fileHandle);
            if (search == folderMetadata.m_fileMap.end())
            {
                folderMetadata.m_fileMap[fileRecord.m_relativeFilePath.string()] = fileHandle;
                folderMetadata.m_fileRecords[fileHandle] = fileRecord;
                metadataUpdated = true;
            }else{
                fileHandle = search->second;
            }
            if (isAsset)
            {
                assetRegistry.AddOrResetFile(fileHandle, fileRecord);
            }
        }
    }
    // 4. If metadata is updated, save metadata.
    if (metadataUpdated)
    {
        folderMetadata.Save(fileMetadataPath);
    }
}
bool ProjectManager::IsInProjectFolder(const std::filesystem::path &target)
{
    auto &projectManager = GetInstance();
    auto projectFolderPath = std::filesystem::absolute(projectManager.m_projectPath.parent_path());
    auto it = std::search(target.begin(), target.end(), projectFolderPath.begin(), projectFolderPath.end());
    return it != target.end();
}
std::filesystem::path ProjectManager::GetRelativePath(const std::filesystem::path &target)
{
    auto &projectManager = GetInstance();
    return std::filesystem::relative(target, projectManager.m_projectPath.parent_path());
}
void ProjectManager::GenerateNewDefaultScene()
{
    auto& projectManager = GetInstance();
    auto scene = AssetManager::CreateAsset<Scene>("New Scene");
    std::filesystem::path newSceneRelativePath = GenerateNewPath("New Scene", ".uescene");
    scene->SetPath(newSceneRelativePath);
    projectManager.m_currentProject->m_startScenePath = newSceneRelativePath;
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
std::filesystem::path ProjectManager::GenerateNewPath(const std::string &filestem, const std::string &extension)
{
    std::filesystem::path retVal = filestem + extension;
    int i = 0;
    while(std::filesystem::exists(retVal)){
        retVal = filestem + "(" + std::to_string(i) + ")" + extension;
    }
    return retVal;
}
void FolderMetadata::Save(const std::filesystem::path &path)
{
    auto directory = path;
    directory.remove_filename();
    std::filesystem::create_directories(directory);
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "m_fileRecords" << YAML::BeginSeq;
    for (const auto &i : m_fileRecords)
    {
        out << YAML::BeginMap;
        {
            out << YAML::Key << "Handle" << YAML::Value << i.first.GetValue();
            i.second.Serialize(out);
        }
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(path.string());
    fout << out.c_str();
    fout.flush();
}
void FolderMetadata::Load(const std::filesystem::path &path)
{
    m_fileMap.clear();
    m_fileRecords.clear();
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    if (in["m_fileRecords"])
    {
        for (const auto &inFileRecords : in["m_fileRecords"])
        {
            Handle handle = Handle(inFileRecords["Handle"].as<uint64_t>());
            FileRecord fileRecord;
            fileRecord.Deserialize(inFileRecords);
            m_fileRecords[handle] = fileRecord;
            m_fileMap[fileRecord.m_relativeFilePath.string()] = handle;
        }
    }
}
void FileRecord::Serialize(YAML::Emitter &out) const
{
    out << YAML::Key << "m_name" << YAML::Value << m_name;
    out << YAML::Key << "m_relativeFilePath" << YAML::Value << m_relativeFilePath.string();
    out << YAML::Key << "m_typeName" << YAML::Value << m_typeName;
}
void FileRecord::Deserialize(const YAML::Node &in)
{
    m_name = in["m_name"].as<std::string>();
    m_relativeFilePath = in["m_relativeFilePath"].as<std::string>();
    m_typeName = in["m_typeName"].as<std::string>();
}