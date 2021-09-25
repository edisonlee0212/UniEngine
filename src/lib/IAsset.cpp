#include <AssetManager.hpp>
#include <DefaultResources.hpp>
#include <IAsset.hpp>
using namespace UniEngine;
bool IAsset::Save()
{
    if (m_projectRelativePath.empty())
        return false;
    auto directory = (ProjectManager::GetProjectPath().parent_path() / m_projectRelativePath).parent_path();
    std::filesystem::create_directories(directory);
    if (SaveInternal(ProjectManager::GetProjectPath().parent_path() / m_projectRelativePath))
    {
        m_saved = true;
        return true;
    }
    return false;
}
bool IAsset::Load()
{
    if (m_projectRelativePath.empty())
        return false;
    if (LoadInternal(ProjectManager::GetProjectPath().parent_path() / m_projectRelativePath))
    {
        m_saved = true;
        return true;
    }
    return false;
}
bool IAsset::SaveInternal(const std::filesystem::path &path)
{
    try
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "m_name" << YAML::Value << m_name;
        Serialize(out);
        out << YAML::EndMap;
        std::ofstream fout(path.string());
        fout << out.c_str();
        fout.flush();
    }
    catch (std::exception e)
    {
        UNIENGINE_ERROR("Failed to save!");
        return false;
    }
    return true;
}
bool IAsset::LoadInternal(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        UNIENGINE_ERROR("Not exist!");
        return false;
    }
    try
    {
        std::ifstream stream(path.string());
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        YAML::Node in = YAML::Load(stringStream.str());
        m_name = in["m_name"].as<std::string>();
        Deserialize(in);
    }
    catch (std::exception e)
    {
        UNIENGINE_ERROR("Failed to load!");
        return false;
    }
    return true;
}

IAsset::~IAsset()
{
}

bool AssetRef::Update()
{
    if (m_assetHandle.GetValue() == 0)
    {
        m_value.reset();
        return false;
    }
    else if (!m_value)
    {
        auto ptr = AssetManager::Get(m_assetHandle);
        if (ptr)
        {
            m_value = ptr;
            m_assetTypeName = ptr->GetTypeName();
            return true;
        }
        Clear();
        return false;
    }
    return true;
}

void AssetRef::Clear()
{
    m_value.reset();
    m_assetHandle = Handle(0);
}
void AssetRef::Set(const AssetRef &target)
{
    m_assetHandle = target.m_assetHandle;
    Update();
}

void IAsset::OnCreate()
{
    m_name = "New " + m_typeName;
}

void IAsset::SetPath(const std::filesystem::path &path)
{
    assert(path.is_relative());
    auto &projectManager = ProjectManager::GetInstance();
    if (path.empty())
    {
        m_projectRelativePath.clear();
        projectManager.m_assetRegistry.RemoveFile(m_handle);
        return;
    }
    m_projectRelativePath = ProjectManager::GetRelativePath(
        std::filesystem::absolute(ProjectManager::GetProjectPath().parent_path() / path));
    m_saved = false;

    if (!projectManager.m_assetRegistry.Find(m_projectRelativePath))
    {
        FileRecord assetRecord;
        assetRecord.m_typeName = m_typeName;
        assetRecord.m_relativeFilePath = m_projectRelativePath;
        projectManager.m_assetRegistry.AddOrResetFile(m_handle, assetRecord);
    }
    else
    {
        projectManager.m_assetRegistry.ResetFilePath(m_handle, m_projectRelativePath);
    }
    auto folder = ProjectManager::FindFolder(m_projectRelativePath.parent_path());
}
bool IAsset::SetPathAndSave(const std::filesystem::path &path)
{
    SetPath(path);
    bool success = Save();
    if(success){
        auto folder = ProjectManager::FindFolder(m_projectRelativePath.parent_path());
        assert(folder);
        ProjectManager::UpdateFolderMetadata(folder);
    }
    return success;
}
bool IAsset::SetPathAndLoad(const std::filesystem::path &path)
{
    auto &projectManager = ProjectManager::GetInstance();
    auto correctedPath = ProjectManager::GetRelativePath(
        std::filesystem::absolute(ProjectManager::GetProjectPath().parent_path() / path));
    if (projectManager.m_assetRegistry.Find(correctedPath, m_handle))
    {
        FileRecord fileRecord;
        projectManager.m_assetRegistry.Find(m_handle, fileRecord);
        m_projectRelativePath = fileRecord.m_relativeFilePath;
        m_name = fileRecord.m_name;
        if (m_typeName == fileRecord.m_typeName)
            return Load();
    }
    return false;
}
bool IAsset::Export(const std::filesystem::path &path)
{
    if (!path.is_absolute())
        return false;
    return SaveInternal(path);
}
