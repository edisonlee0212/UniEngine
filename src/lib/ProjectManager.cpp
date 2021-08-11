#include "ProjectManager.hpp"
#include <AssetManager.hpp>
using namespace UniEngine;

void ProjectManager::SetProjectPath(const std::filesystem::path &path)
{
    GetInstance().m_projectPath = path;
    AssetManager::GetAssetFolderPath();
}

std::filesystem::path ProjectManager::GetProjectPath()
{
    auto &path = GetInstance().m_projectPath;
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    return path;
}