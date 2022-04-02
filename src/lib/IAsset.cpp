#include <DefaultResources.hpp>
#include <IAsset.hpp>
#include <ProjectManager.hpp>
using namespace UniEngine;
bool IAsset::Save()
{
    if (IsTemporary())
        return false;
    auto path = GetAbsolutePath();
    if (SaveInternal(path))
    {
        m_saved = true;
        return true;
    }
    return false;
}
bool IAsset::Load()
{
    if (IsTemporary())
        return false;
    auto path = GetAbsolutePath();
    if (LoadInternal(path))
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
        Serialize(out);
        out << YAML::EndMap;
        std::ofstream fout(path.string());
        fout << out.c_str();
        fout.close();
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
        Deserialize(in);
    }
    catch (std::exception e)
    {
        UNIENGINE_ERROR("Failed to load!");
        return false;
    }
    return true;
}

void IAsset::OnCreate()
{
}

bool IAsset::Export(const std::filesystem::path &path)
{
    if (ProjectManager::IsInProjectFolder(path))
    {
        UNIENGINE_ERROR("Path is in project folder!");
        return false;
    }
    return SaveInternal(path);
}
bool IAsset::Import(const std::filesystem::path &path)
{
    if (!ProjectManager::GetProjectPath().empty() && ProjectManager::IsInProjectFolder(path))
    {
        UNIENGINE_ERROR("Path is in project folder!");
        return false;
    }
    return LoadInternal(path);
}

void IAsset::SetUnsaved()
{
    m_saved = true;
}
bool IAsset::Saved() const
{
    return m_saved;
}
bool IAsset::IsTemporary() const
{
    return m_assetRecord.expired();
}
std::weak_ptr<AssetRecord> IAsset::GetAssetRecord() const
{
    return m_assetRecord;
}
std::filesystem::path IAsset::GetProjectRelativePath() const
{
    if (m_assetRecord.expired())
        return {};
    return m_assetRecord.lock()->GetProjectRelativePath();
}
std::filesystem::path IAsset::GetAbsolutePath() const
{
    if (m_assetRecord.expired())
        return {};
    return m_assetRecord.lock()->GetAbsolutePath();
}
bool IAsset::SetPathAndSave(const std::filesystem::path &projectRelativePath)
{
    if (!projectRelativePath.is_relative())
    {
        UNIENGINE_ERROR("Not relative path!");
        return false;
    }
    if (std::filesystem::exists(ProjectManager::GetProjectPath().parent_path() / projectRelativePath))
    {
        return false;
    }
    if (ProjectManager::IsValidAssetFileName(projectRelativePath))
    {
        UNIENGINE_ERROR("Asset path invalid!");
        return false;
    }
    auto newFolder = ProjectManager::GetOrCreateFolder(projectRelativePath.parent_path()).lock();
    if (!IsTemporary())
    {
        auto assetRecord = m_assetRecord.lock();
        auto folder = assetRecord->GetFolder().lock();
        if (newFolder == folder)
        {
            assetRecord->SetAssetFileName(projectRelativePath.stem().string());
        }
        else
        {
            folder->MoveAsset(m_handle, newFolder);
        }
    }
    else
    {
        auto stem = projectRelativePath.stem().string();
        auto fileName = projectRelativePath.filename().string();
        auto extension = projectRelativePath.extension().string();
        if (fileName == stem)
        {
            stem = "";
            extension = fileName;
        }
        newFolder->RegisterAsset(m_self.lock(), stem, extension);
    }
    Save();
    return true;
}
std::string IAsset::GetTitle() const
{
    return IsTemporary() ? "Temporary " + m_typeName
                         : (GetProjectRelativePath().stem().string() + (m_saved ? "" : " *"));
}
IAsset::~IAsset()
{
    auto &projectManager = ProjectManager::GetInstance();
    projectManager.m_assetRegistry.erase(m_handle);
}
