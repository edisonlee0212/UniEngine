#include <AssetManager.hpp>
#include <DefaultResources.hpp>
#include <IAsset.hpp>
using namespace UniEngine;
bool IAsset::Save()
{
    if (m_projectRelativePath.empty())
        return false;
    if(SaveInternal(ProjectManager::GetProjectPath().parent_path() / m_projectRelativePath)) {
        m_saved = true;
        return true;
    }
    return false;
}
bool IAsset::Load()
{
    if (m_projectRelativePath.empty())
        return false;
    if(LoadInternal(ProjectManager::GetProjectPath().parent_path() / m_projectRelativePath)) {
        m_saved = true;
        return true;
    }
    return false;
}
bool IAsset::SaveInternal(const std::filesystem::path &path)
{
    try
    {
        auto directory = path;
        directory.remove_filename();
        std::filesystem::create_directories(directory);
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "m_name" << YAML::Value << m_name;
        Serialize(out);
        out << YAML::EndMap;
        std::ofstream fout(path.string());
        fout << out.c_str();
        fout.flush();
    }catch (std::exception e){
        UNIENGINE_ERROR("Failed to save!");
        return false;
    }
    return true;
}
bool IAsset::LoadInternal(const std::filesystem::path &path)
{
    if(!std::filesystem::exists(path)){
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
    }catch (std::exception e){
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
    if(path.empty()){
        return;
    }
    m_projectRelativePath = ProjectManager::GetRelativePath(std::filesystem::absolute(ProjectManager::GetProjectPath().parent_path() / path));
    m_saved = false;
    auto &assetRecords = ProjectManager::GetInstance().m_assetRegistry->m_assetRecords;
    auto &fileMap = ProjectManager::GetInstance().m_assetRegistry->m_fileMap;
    auto search = fileMap.find(m_projectRelativePath.string());
    if (search != fileMap.end())
    {
        m_handle = search->second;
    }
    else
    {
        FileRecord assetRecord;
        assetRecord.m_typeName = m_typeName;
        assetRecord.m_relativeFilePath = m_projectRelativePath;
        assetRecords[m_handle] = assetRecord;
        fileMap[m_projectRelativePath.string()] = m_handle;
    }
}
bool IAsset::SetPathAndSave(const std::filesystem::path &path)
{
    SetPath(path);
    return Save();
}
bool IAsset::SetPathAndLoad(const std::filesystem::path &path)
{
    SetPath(path);
    return Load();
}