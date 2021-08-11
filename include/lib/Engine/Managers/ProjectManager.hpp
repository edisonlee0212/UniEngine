#pragma once
#include <ISingleton.hpp>
#include <IAsset.hpp>

namespace UniEngine
{
class UNIENGINE_API ProjectManager : public ISingleton<ProjectManager>
{
    friend class AssetManager;
    std::filesystem::path m_projectPath;
  public:
    static void SetProjectPath(const std::filesystem::path &path);
    static std::filesystem::path GetProjectPath();
};
} // namespace UniEngine