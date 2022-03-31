#include "ProjectManager.hpp"
#include "Engine/ECS/Entities.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include <Application.hpp>
#include <AssetManager.hpp>
#include <PhysicsLayer.hpp>

using namespace UniEngine;

std::shared_ptr<IAsset> AssetRecord::GetAsset() const
{
    if (!m_asset.expired())
        return m_asset.lock();
    if (!m_assetTypeName.empty() && m_assetHandle != 0)
    {
        size_t hashCode;
        auto retVal = std::dynamic_pointer_cast<IAsset>(
            Serialization::ProduceSerializable(m_assetTypeName, hashCode, m_assetHandle));
        auto self = m_folder.lock()->GetAssetRecord(m_assetHandle);
        retVal->m_assetRecord = self;
        retVal->OnCreate();
        if (std::filesystem::exists(GetAbsolutePath()))
        {
            retVal->Load();
        }
        else
        {
            retVal->Save();
        }
        return retVal;
    }
    return nullptr;
}
std::string AssetRecord::GetAssetTypeName() const
{
    return m_assetTypeName;
}
std::string AssetRecord::GetAssetFileName() const
{
    return m_assetFileName;
}
std::string AssetRecord::GetAssetExtension() const
{
    return m_assetExtension;
}
std::filesystem::path AssetRecord::GetProjectRelativePath() const
{
    if (m_folder.expired())
    {
        UNIENGINE_ERROR("Folder expired!");
        return {};
    }
    return m_folder.lock()->GetProjectRelativePath() / (m_assetFileName + m_assetExtension);
}
std::filesystem::path AssetRecord::GetAbsolutePath() const
{
    if (m_folder.expired())
    {
        UNIENGINE_ERROR("Folder expired!");
        return {};
    }
    return m_folder.lock()->GetAbsolutePath() / (m_assetFileName + m_assetExtension);
}
void AssetRecord::SetAssetFileName(const std::string &newName)
{
    // TODO: Check invalid filename.
    auto oldPath = GetAbsolutePath();
    auto newPath = oldPath;
    newPath.replace_filename(newName);
    if (std::filesystem::exists(newPath))
    {
        UNIENGINE_ERROR("File with new name already exists!");
        return;
    }
    DeleteMetadata();
    m_assetFileName = newName;
    if (std::filesystem::exists(oldPath))
    {
        std::filesystem::rename(oldPath, newPath);
    }
    Save();
}
void AssetRecord::SetAssetExtension(const std::string &newExtension)
{
    auto validExtensions = AssetManager::GetExtension();
    bool found = false;
    for (const auto &i : validExtensions)
    {
        if (i == newExtension)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        UNIENGINE_ERROR("Extension not valid!");
        return;
    }
    auto oldPath = GetAbsolutePath();
    auto newPath = oldPath;
    newPath.replace_extension(newExtension);
    if (std::filesystem::exists(newPath))
    {
        UNIENGINE_ERROR("File with new name already exists!");
        return;
    }
    DeleteMetadata();
    m_assetExtension = newExtension;
    if (std::filesystem::exists(oldPath))
    {
        std::filesystem::rename(oldPath, newPath);
    }
    Save();
}
void AssetRecord::Save() const
{
    auto path = GetAbsolutePath().replace_extension(".uemetadata");
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "m_assetExtension" << YAML::Value << m_assetExtension;
    out << YAML::Key << "m_assetTypeName" << YAML::Value << m_assetTypeName;
    out << YAML::Key << "m_assetHandle" << YAML::Value << m_assetHandle;
    out << YAML::EndMap;
    std::ofstream fout(path.string());
    fout << out.c_str();
    fout.close();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    DWORD attributes = GetFileAttributes(path.string().c_str());
    SetFileAttributes(path.string().c_str(), attributes | FILE_ATTRIBUTE_HIDDEN);
#endif
}

Handle AssetRecord::GetAssetHandle() const
{
    return m_assetHandle;
}
void AssetRecord::DeleteMetadata() const
{
    auto path = GetAbsolutePath().replace_extension(".uemetadata");
    std::filesystem::remove(path);
}
void AssetRecord::Load(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        UNIENGINE_ERROR("Metadata not exist!");
        return;
    }
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    if (in["m_assetExtension"])
        m_assetExtension = in["m_assetExtension"].as<std::string>();
    if (in["m_assetTypeName"])
        m_assetTypeName = in["m_assetTypeName"].as<std::string>();
    if (in["m_assetHandle"])
        m_assetHandle = in["m_assetHandle"].as<uint64_t>();
}
std::filesystem::path Folder::GetProjectRelativePath() const
{
    return m_parent.lock()->GetProjectRelativePath() / m_name;
}
std::filesystem::path Folder::GetAbsolutePath() const
{
    return Application::GetProjectManager().m_projectPath / GetProjectRelativePath();
}

Handle Folder::GetHandle() const
{
    return m_handle;
}
std::string Folder::GetName() const
{
    return m_name;
}
void Folder::Rename(const std::string &newName)
{
    auto oldPath = GetAbsolutePath();
    auto newPath = oldPath;
    newPath.replace_filename(newName);
    if (std::filesystem::exists(newPath))
    {
        UNIENGINE_ERROR("Folder with new name already exists!");
        return;
    }
    DeleteMetadata();
    m_name = newName;
    if (std::filesystem::exists(oldPath))
    {
        std::filesystem::rename(oldPath, newPath);
    }
    Save();
}
void Folder::Save() const
{
    auto path = GetAbsolutePath().replace_extension(".uefoldermetadata");
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "m_handle" << YAML::Value << m_handle;
    out << YAML::EndMap;
    std::ofstream fout(path.string());
    fout << out.c_str();
    fout.close();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    DWORD attributes = GetFileAttributes(path.string().c_str());
    SetFileAttributes(path.string().c_str(), attributes | FILE_ATTRIBUTE_HIDDEN);
#endif
}
void Folder::Load(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        UNIENGINE_ERROR("Folder metadata not exist!");
        return;
    }
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    if (in["m_handle"])
        m_handle = in["m_handle"].as<uint64_t>();
}
void Folder::DeleteMetadata() const
{
    auto path = GetAbsolutePath().replace_extension(".uefoldermetadata");
    std::filesystem::remove(path);
}
void Folder::MoveChild(const Handle &childHandle, const std::shared_ptr<Folder> &dest)
{
    auto search = m_children.find(childHandle);
    if (search == m_children.end())
    {
        UNIENGINE_ERROR("Child not exist!");
        return;
    }
    auto child = search->second;
    auto newPath = dest->GetAbsolutePath() / child->GetName();
    if (std::filesystem::exists(newPath))
    {
        UNIENGINE_ERROR("Destination folder already exists!");
        return;
    }
    auto oldPath = child->GetAbsolutePath();
    child->DeleteMetadata();
    m_children.erase(childHandle);
    if (std::filesystem::exists(oldPath))
    {
        std::filesystem::rename(oldPath, newPath);
    }
    dest->m_children.insert({childHandle, child});
    child->m_parent = dest;
    child->Save();
}
std::weak_ptr<Folder> Folder::GetChild(const Handle &childHandle)
{
    auto search = m_children.find(childHandle);
    if (search == m_children.end())
    {
        return {};
    }
    return search->second;
}
std::weak_ptr<Folder> Folder::GetOrCreateChild(const std::string &folderName)
{
    for (const auto &i : m_children)
    {
        if (i.second->m_name == folderName)
            return i.second;
    }
    auto newFolder = std::make_shared<Folder>();
    newFolder->m_name = folderName;
    newFolder->m_handle = Handle();
    m_children[newFolder->m_handle] = newFolder;
    newFolder->m_parent = m_parent.lock()->GetChild(m_handle);
    newFolder->Save();
    return newFolder;
}
void Folder::DeleteChild(const Handle &childHandle)
{
    auto child = GetChild(childHandle).lock();
    child->DeleteMetadata();
    m_children.erase(childHandle);
}
std::weak_ptr<AssetRecord> Folder::GetOrCreateAssetRecord(const std::string &fileName, const std::string &extension)
{
    auto typeName = AssetManager::GetTypeName(extension);
    if (typeName.empty())
    {
        UNIENGINE_ERROR("Asset type not exist!");
        return {};
    }
    for (const auto &i : m_assetRecords)
    {
        if (i.second->m_assetFileName == fileName && i.second->m_assetExtension == extension)
            return i.second;
    }
    auto record = std::make_shared<AssetRecord>();
    record->m_folder = m_parent.lock()->GetChild(m_handle);
    record->m_assetTypeName = typeName;
    record->m_assetExtension = extension;
    record->m_assetFileName = fileName;
    record->m_assetHandle = Handle();
    m_assetRecords[record->m_assetHandle] = record;
    auto asset = record->GetAsset();
    record->Save();
    return record;
}
std::weak_ptr<AssetRecord> Folder::GetAssetRecord(const Handle &assetHandle)
{
    auto search = m_assetRecords.find(assetHandle);
    if (search != m_assetRecords.end())
    {
        return search->second;
    }
    return {};
}
void Folder::MoveAssetRecord(const Handle &assetHandle, const std::shared_ptr<Folder> &dest)
{
    auto search = m_assetRecords.find(assetHandle);
    if (search == m_assetRecords.end())
    {
        UNIENGINE_ERROR("AssetRecord not exist!");
        return;
    }
    auto assetRecord = search->second;
    auto newPath = dest->GetAbsolutePath() / (assetRecord->m_assetFileName + assetRecord->m_assetExtension);
    if (std::filesystem::exists(newPath))
    {
        UNIENGINE_ERROR("Destination file already exists!");
        return;
    }
    auto oldPath = assetRecord->GetAbsolutePath();
    assetRecord->DeleteMetadata();
    m_assetRecords.erase(assetHandle);
    if (std::filesystem::exists(oldPath))
    {
        std::filesystem::rename(oldPath, newPath);
    }
    dest->m_assetRecords.insert({assetHandle, assetRecord});
    assetRecord->m_folder = dest;
    assetRecord->Save();
}
void Folder::DeleteAssetRecord(const Handle &assetHandle)
{
    auto assetRecord = GetAssetRecord(assetHandle).lock();
    assetRecord->DeleteMetadata();
    m_children.erase(assetHandle);
}
void Folder::Refresh(const std::filesystem::path &parentAbsolutePath)
{
    auto path = parentAbsolutePath / m_name;
    /**
     * 1. Scan folder for any unregistered folders and assets.
     */
    std::vector<std::filesystem::path> childFolderMetadataList;
    std::vector<std::filesystem::path> childFolderList;
    std::vector<std::filesystem::path> assetMetadataList;
    std::vector<std::filesystem::path> fileList;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (std::filesystem::is_directory(entry.path()))
        {
            childFolderList.push_back(entry.path());
        }
        else if (entry.path().extension() == ".uefoldermetadata")
        {
            childFolderMetadataList.push_back(entry.path());
        }
        else if (entry.path().extension() == ".uemetadata")
        {
            assetMetadataList.push_back(entry.path());
        }
        else
        {
            fileList.push_back(entry.path());
        }
    }
    for (const auto &childFolderMetadataPath : childFolderMetadataList)
    {
        auto childFolderPath = childFolderMetadataPath;
        childFolderPath.replace_extension("");
        if (!std::filesystem::exists(childFolderPath))
        {
            std::filesystem::remove(childFolderMetadataPath);
        }
        else
        {
            auto folderName = childFolderMetadataPath.filename();
            folderName.replace_extension("");
            std::shared_ptr<Folder> child;
            for (const auto &i : m_children)
            {
                if (i.second->m_name == folderName)
                {
                    child = i.second;
                }
            }
            if (!child)
            {
                auto newFolder = std::make_shared<Folder>();
                newFolder->m_name = folderName.string();
                newFolder->m_parent = m_parent.lock()->GetChild(m_handle);
                newFolder->Load(childFolderMetadataPath);
                m_children[newFolder->m_handle] = newFolder;
            }
        }
    }
    for (const auto &childFolderPath : childFolderList)
    {
        auto childFolder = GetOrCreateChild(childFolderPath.filename().string()).lock();
        childFolder->Refresh(path);
    }
    for (const auto &assetMetadataPath : assetMetadataList)
    {
        auto assetName = assetMetadataPath.filename();
        assetName.replace_extension("");
        std::shared_ptr<AssetRecord> assetRecord;
        for (const auto &i : m_assetRecords)
        {
            if (i.second->m_assetFileName == assetName)
            {
                assetRecord = i.second;
            }
        }

        if (!assetRecord)
        {
            auto newAssetRecord = std::make_shared<AssetRecord>();
            newAssetRecord->m_assetFileName = assetName.string();
            newAssetRecord->m_folder = m_parent.lock()->GetChild(m_handle);
            newAssetRecord->Load(assetMetadataPath);
            if(!std::filesystem::exists(newAssetRecord->GetAbsolutePath())){
                std::filesystem::remove(assetMetadataPath);
            }
        }
    }
    for (const auto &filePath : fileList)
    {
        auto filename = filePath.filename().string();
        auto extension = filePath.extension().string();
        auto typeName = AssetManager::GetTypeName(extension);
        if (!typeName.empty())
        {
            GetOrCreateAssetRecord(filename, extension);
        }
    }
    /**
     * 2. Clear deleted asset and folder.
     */
    std::vector<Handle> assetToRemove;
    for (const auto &i : m_assetRecords)
    {
        if (std::filesystem::exists(i.second->GetAbsolutePath()))
        {
            assetToRemove.push_back(i.first);
        }
    }
    for (const auto &i : assetToRemove)
    {
        DeleteAssetRecord(i);
    }
    std::vector<Handle> folderToRemove;
    for (const auto &i : m_children)
    {
        if (std::filesystem::exists(i.second->GetAbsolutePath()))
        {
            folderToRemove.push_back(i.first);
        }
    }
    for (const auto &i : folderToRemove)
    {
        DeleteChild(i);
    }
}
