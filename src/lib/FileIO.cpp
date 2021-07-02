#include <Gui.hpp>
#include <ImGuiFileBrowser.hpp>
#include <Debug.hpp>
#include <FileIO.hpp>
#include <WindowManager.hpp>
using namespace UniEngine;

std::string FileIO::GetAssetFolderPath()
{
    std::string path = GetProjectPath() + "Assets/";
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    return path;
}

void FileIO::SetProjectPath(const std::string &path)
{
    GetInstance().m_projectPath = std::make_unique<std::string>(path);
}

std::string FileIO::GetProjectPath()
{
    std::string path;
    if (!GetInstance().m_projectPath)
        path = GetResourcePath();
    else
        path = *GetInstance().m_projectPath;
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    return path;
}

void FileIO::SetResourcePath(const std::string &path)
{
    GetInstance().m_resourceRootPath = std::make_unique<std::string>(path);
}

std::string FileIO::GetResourcePath(const std::string &path)
{
    return *GetInstance().m_resourceRootPath + path;
}

std::string FileIO::LoadFileAsString(const std::string &path)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        file.open(path);
        std::stringstream stream;
        // read file's buffer contents into streams
        stream << file.rdbuf();
        // close file handlers
        file.close();
        // convert stream into string
        return stream.str();
    }
    catch (std::ifstream::failure e)
    {
        UNIENGINE_ERROR("Load file failed!")
        throw;
    }
}

void FileIO::OpenFile(
    const std::string &dialogTitle,
    const std::string &filters,
    const std::function<void(const std::string &filePath)> &func
)
{
    if (ImGui::Button(dialogTitle.c_str()))
        ImGui::OpenPopup(dialogTitle.c_str());
    static imgui_addons::ImGuiFileBrowser file_dialog;
    if (file_dialog.showFileDialog(
            dialogTitle, imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), filters))
    {
        func(file_dialog.selected_path);
    }  
}

void FileIO::SaveFile(
    const std::string &dialogTitle,
    const std::string &filters,
    const std::function<void(const std::string &filePath)> &func)
{
    if (ImGui::Button(dialogTitle.c_str()))
        ImGui::OpenPopup(dialogTitle.c_str());
    static imgui_addons::ImGuiFileBrowser file_dialog;
    if (file_dialog.showFileDialog(
            dialogTitle, imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), filters))
    {
        func(file_dialog.selected_path);
    }
}

std::pair<bool, uint32_t> FileIO::DirectoryTreeViewRecursive(
    const std::filesystem::path &path, uint32_t *count, int *selection_mask)
{
    const ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;

    bool anyNodeClicked = false;
    uint32_t nodeClicked = 0;

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        ImGuiTreeNodeFlags nodeFlags = baseFlags;
        const bool isSelected = (*selection_mask & (1 << (*count))) != 0;
        if (isSelected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        std::string name = entry.path().string();

        auto lastSlash = name.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        name = name.substr(lastSlash, name.size() - lastSlash);

        const bool entryIsFile = !std::filesystem::is_directory(entry.path());
        if (entryIsFile)
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        const bool nodeOpen =
            ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<intptr_t>(*count)), nodeFlags, name.c_str());

        if (ImGui::IsItemClicked())
        {
            nodeClicked = *count;
            anyNodeClicked = true;
        }

        (*count)--;

        if (!entryIsFile)
        {
            if (nodeOpen)
            {
                const auto clickState = DirectoryTreeViewRecursive(entry.path(), count, selection_mask);

                if (!anyNodeClicked)
                {
                    anyNodeClicked = clickState.first;
                    nodeClicked = clickState.second;
                }

                ImGui::TreePop();
            }
            else
            {
                for (const auto &e : std::filesystem::recursive_directory_iterator(entry.path()))
                    (*count)--;
            }
        }
    }

    return {anyNodeClicked, nodeClicked};
}
