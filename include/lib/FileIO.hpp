#pragma once
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API FileIO : ISingleton<FileIO>
{
    std::unique_ptr<std::string> m_projectPath;
    std::unique_ptr<std::string> m_resourceRootPath;
    friend class Application;
    static void SetResourcePath(const std::string &path);

  public:
    static std::string GetAssetFolderPath();
    static void SetProjectPath(const std::string &path);
    static std::string GetProjectPath();
    static std::string GetResourcePath(const std::string &path = "");
    static std::string LoadFileAsString(const std::string &path = "");
    static void OpenFile(
        const std::string &dialogTitle,
        const std::string &filters,
        const std::function<void(const std::string &filePath)> &func);
    static void SaveFile(
        const std::string &dialogTitle,
        const std::string &filters,
        const std::function<void(const std::string &filePath)> &func);
    static std::pair<bool, uint32_t> DirectoryTreeViewRecursive(
        const std::filesystem::path &path, uint32_t *count, int *selection_mask);
};
} // namespace UniEngine