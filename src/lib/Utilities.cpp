#include "Windows.hpp"
#include "Entities.hpp"
#include "Console.hpp"
#include "shlobj.h"
#include <ImGuiFileBrowser.hpp>
#include <ProjectManager.hpp>
#include <Utilities.hpp>
using namespace UniEngine;
Bound::Bound()
{
    m_min = glm::vec3(FLT_MIN);
    m_min = glm::vec3(FLT_MIN);
    m_max = glm::vec3(FLT_MAX);
}

glm::vec3 Bound::Size() const
{
    return (m_max - m_min) / 2.0f;
}

glm::vec3 Bound::Center() const
{
    return (m_max + m_min) / 2.0f;
}

bool Bound::InBound(const glm::vec3 &position) const
{
    glm::vec3 center = (m_min + m_max) / 2.0f;
    glm::vec3 size = (m_max - m_min) / 2.0f;
    if (glm::abs(position.x - center.x) > size.x)
        return false;
    if (glm::abs(position.y - center.y) > size.y)
        return false;
    if (glm::abs(position.z - center.z) > size.z)
        return false;
    return true;
}

void Bound::ApplyTransform(const glm::mat4 &transform)
{
    std::vector<glm::vec3> corners;
    PopulateCorners(corners);
    m_min = glm::vec3(FLT_MAX);
    m_max = glm::vec3(FLT_MIN);

    // Transform all of the corners, and keep track of the greatest and least
    // values we see on each coordinate axis.
    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed = transform * glm::vec4(corners[i], 1.0f);
        m_min = (glm::min)(m_min, transformed);
        m_max = (glm::max)(m_max, transformed);
    }
}

void Bound::PopulateCorners(std::vector<glm::vec3> &corners) const
{
    corners.resize(8);
    corners[0] = m_min;
    corners[1] = glm::vec3(m_min.x, m_min.y, m_max.z);
    corners[2] = glm::vec3(m_min.x, m_max.y, m_min.z);
    corners[3] = glm::vec3(m_max.x, m_min.y, m_min.z);
    corners[4] = glm::vec3(m_min.x, m_max.y, m_max.z);
    corners[5] = glm::vec3(m_max.x, m_min.y, m_max.z);
    corners[6] = glm::vec3(m_max.x, m_max.y, m_min.z);
    corners[7] = m_max;
}
Bezier2D::Bezier2D()
{
    m_controlPoints[0] = glm::vec2(0);
    m_controlPoints[1] = glm::vec2(0.5f, 0.0f);
    m_controlPoints[2] = glm::vec2(0.5f, 1.0f);
    m_controlPoints[3] = glm::vec2(1, 1);
}
glm::vec2 Bezier2D::GetPoint(const float &t) const
{
    float t1 = 1.0f - t;

    return t1 * t1 * t1 * m_controlPoints[0] + 3.0f * t1 * t1 * t * m_controlPoints[1] +
           3.0f * t1 * t * t * m_controlPoints[2] + t * t * t * m_controlPoints[3];
}
bool Bezier2D::DrawGraph(const std::string &label)
{

    // visuals
    enum
    {
        SMOOTHNESS = 64
    }; // curve smoothness: the higher number of segments, the smoother curve
    enum
    {
        CURVE_WIDTH = 4
    }; // main curved line width
    enum
    {
        LINE_WIDTH = 1
    }; // handlers: small lines width
    enum
    {
        GRAB_RADIUS = 8
    }; // handlers: circle radius
    enum
    {
        GRAB_BORDER = 2
    }; // handlers: circle border width
    enum
    {
        AREA_CONSTRAINED = true
    }; // should grabbers be constrained to grid area?
    enum
    {
        AREA_WIDTH = 0
    }; // area width in pixels. 0 for adaptive size (will use max avail width)

    const ImGuiStyle &Style = ImGui::GetStyle();
    const ImGuiIO &IO = ImGui::GetIO();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    ImGuiWindow *Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems)
        return false;
    bool changed = false;
    // header and spacing
    if (ImGui::TreeNode(label.c_str()))
    {
        if (!m_fixed && ImGui::SliderFloat2("P0", &m_controlPoints[0].x, 0, 1, "%.3f", 1.0f))
            changed = true;
        if (ImGui::SliderFloat2("P1", &m_controlPoints[1].x, 0, 1, "%.3f", 1.0f))
            changed = true;
        if (ImGui::SliderFloat2("P2", &m_controlPoints[2].x, 0, 1, "%.3f", 1.0f))
            changed = true;
        if (!m_fixed && ImGui::SliderFloat2("P3", &m_controlPoints[3].x, 0, 1, "%.3f", 1.0f))
            changed = true;
        int hovered = ImGui::IsItemActive() || ImGui::IsItemHovered(); // IsItemDragged() ?
        ImGui::Dummy(ImVec2(0, 3));

        // prepare canvas
        const float avail = ImGui::GetWindowContentRegionWidth();
        const float dim = AREA_WIDTH > 0 ? AREA_WIDTH : avail;
        ImVec2 Canvas(dim, dim);

        ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, NULL))
        {
            ImGui::TreePop();
            return changed;
        }

        const ImGuiID id = Window->GetID(label.c_str());
        hovered |= 0 != ImGui::ItemHoverable(ImRect(bb.Min, bb.Min + ImVec2(avail, dim)), id);

        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

        // background grid
        for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4))
        {
            DrawList->AddLine(
                ImVec2(bb.Min.x + i, bb.Min.y),
                ImVec2(bb.Min.x + i, bb.Max.y),
                ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }
        for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4))
        {
            DrawList->AddLine(
                ImVec2(bb.Min.x, bb.Min.y + i),
                ImVec2(bb.Max.x, bb.Min.y + i),
                ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }

        // eval curve
        ImVec2 Q[4] = {
            {m_controlPoints[0].x, m_controlPoints[0].y},
            {m_controlPoints[1].x, m_controlPoints[1].y},
            {m_controlPoints[2].x, m_controlPoints[2].y},
            {m_controlPoints[3].x, m_controlPoints[3].y}};

        // control points: 2 lines and 2 circles
        {
            // handle grabbers
            ImVec2 mouse = ImGui::GetIO().MousePos, pos[2];
            float distance[2];

            for (int i = 1; i < 3; ++i)
            {
                pos[i - 1] = ImVec2(m_controlPoints[i].x, 1 - m_controlPoints[i].y) * (bb.Max - bb.Min) + bb.Min;
                distance[i - 1] = (pos[i - 1].x - mouse.x) * (pos[i - 1].x - mouse.x) +
                                  (pos[i - 1].y - mouse.y) * (pos[i - 1].y - mouse.y);
            }

            int selected = distance[0] < distance[1] ? 1 : 2;
            if (distance[selected - 1] < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS))
            {
                ImGui::SetTooltip("(%4.3f, %4.3f)", m_controlPoints[selected].x, m_controlPoints[selected].y);

                if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))
                {
                    float &px = m_controlPoints[selected].x += ImGui::GetIO().MouseDelta.x / Canvas.x;
                    float &py = m_controlPoints[selected].y -= ImGui::GetIO().MouseDelta.y / Canvas.y;
                    if (AREA_CONSTRAINED)
                    {
                        px = (px < 0 ? 0 : (px > 1 ? 1 : px));
                        py = (py < 0 ? 0 : (py > 1 ? 1 : py));
                    }

                    changed = true;
                }
            }
        }

        // if (hovered || changed) DrawList->PushClipRectFullScreen();
        std::vector<glm::vec2> results;
        results.resize(SMOOTHNESS + 1);
        for (int i = 0; i < SMOOTHNESS + 1; i++)
        {
            results[i] = GetPoint(static_cast<float>(i) / SMOOTHNESS);
        }
        // draw curve
        {
            ImColor color(ImGui::GetStyle().Colors[ImGuiCol_PlotLines]);
            for (int i = 0; i < SMOOTHNESS; ++i)
            {
                ImVec2 p = {results[i + 0].x, 1 - results[i + 0].y};
                ImVec2 q = {results[i + 1].x, 1 - results[i + 1].y};
                ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                DrawList->AddLine(r, s, color, CURVE_WIDTH);
            }
        }

        // draw preview (cycles every 1s)
        static clock_t epoch = clock();
        ImVec4 white(ImGui::GetStyle().Colors[ImGuiCol_Text]);
        for (int i = 0; i < 3; ++i)
        {
            double now = ((clock() - epoch) / (double)CLOCKS_PER_SEC);
            float delta = ((int)(now * 1000) % 1000) / 1000.f;
            delta += i / 3.f;
            if (delta > 1)
                delta -= 1;
            int idx = (int)(delta * SMOOTHNESS);
            float evalx = results[idx].x; //
            float evaly = results[idx].y; // ImGui::BezierValue( delta, P );
            ImVec2 p0 = ImVec2(evalx, 1 - 0) * (bb.Max - bb.Min) + bb.Min;
            ImVec2 p1 = ImVec2(0, 1 - evaly) * (bb.Max - bb.Min) + bb.Min;
            ImVec2 p2 = ImVec2(evalx, 1 - evaly) * (bb.Max - bb.Min) + bb.Min;
            DrawList->AddCircleFilled(p0, GRAB_RADIUS / 2, ImColor(white));
            DrawList->AddCircleFilled(p1, GRAB_RADIUS / 2, ImColor(white));
            DrawList->AddCircleFilled(p2, GRAB_RADIUS / 2, ImColor(white));
        }

        // draw lines and grabbers
        float luma = ImGui::IsItemActive() || ImGui::IsItemHovered() ? 0.5f : 1.0f;
        ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
        ImVec2 p0 = ImVec2(m_controlPoints[0].x, 1 - m_controlPoints[0].y) * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p1 = ImVec2(m_controlPoints[1].x, 1 - m_controlPoints[1].y) * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p2 = ImVec2(m_controlPoints[2].x, 1 - m_controlPoints[2].y) * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p3 = ImVec2(m_controlPoints[3].x, 1 - m_controlPoints[3].y) * (bb.Max - bb.Min) + bb.Min;
        DrawList->AddLine(p0, p1, ImColor(white), LINE_WIDTH);
        DrawList->AddLine(p3, p2, ImColor(white), LINE_WIDTH);
        DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
        DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
        DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
        DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));

        // if (hovered || changed) DrawList->PopClipRect();
        ImGui::TreePop();
    }
    return changed;
}
Ray::Ray(glm::vec3 start, glm::vec3 end)
{
    m_start = start;
    m_direction = glm::normalize(end - start);
    m_length = glm::distance(start, end);
}

Ray::Ray(glm::vec3 start, glm::vec3 direction, float length)
{
    m_start = start;
    m_direction = direction;
    m_length = length;
}

bool Ray::Intersect(const glm::vec3 &position, float radius) const
{
    const glm::vec3 rayEnd = m_start + m_direction * m_length;
    const auto cp = glm::closestPointOnLine(position, m_start, rayEnd);
    if (cp == m_start || cp == rayEnd)
        return false;
    return glm::distance(cp, position) <= radius;
}

bool Ray::Intersect(const glm::mat4 &model, const Bound &bound) const
{
    float tMin = 0.0f;
    float tMax = 100000.0f;
    GlobalTransform t;
    t.m_value = model;
    glm::vec3 scale = t.GetScale();
    t.SetScale(glm::vec3(1.0f));
    glm::mat4 transform = t.m_value;

    glm::vec3 OBBWorldSpace(transform[3].x, transform[3].y, transform[3].z);

    glm::vec3 delta = OBBWorldSpace - m_start;
    glm::vec3 AABBMin = scale * (bound.m_min);
    glm::vec3 AABBMax = scale * (bound.m_max);
    // Test intersection with the 2 planes perpendicular to the OBB's X axis
    {
        glm::vec3 xAxis(transform[0].x, transform[0].y, transform[0].z);

        float e = glm::dot(xAxis, delta);
        float f = glm::dot(m_direction, xAxis);

        if (fabs(f) > 0.001f)
        { // Standard case

            float t1 = (e + AABBMin.x) / f; // Intersection with the "left" plane
            float t2 = (e + AABBMax.x) / f; // Intersection with the "right" plane
            // t1 and t2 now contain distances betwen ray origin and ray-plane intersections

            // We want t1 to represent the nearest intersection,
            // so if it's not the case, invert t1 and t2
            if (t1 > t2)
            {
                float w = t1;
                t1 = t2;
                t2 = w; // swap t1 and t2
            }

            // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
            if (t2 < tMax)
                tMax = t2;
            // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
            if (t1 > tMin)
                tMin = t1;

            // And here's the trick :
            // If "far" is closer than "near", then there is NO intersection.
            // See the images in the tutorials for the visual explanation.
            if (tMax < tMin)
                return false;
        }
        else
        { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
            if (-e + AABBMin.x > 0.0f || -e + AABBMax.x < 0.0f)
                return false;
        }
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Y axis
    // Exactly the same thing than above.
    {
        glm::vec3 yAxis(transform[1].x, transform[1].y, transform[1].z);
        float e = glm::dot(yAxis, delta);
        float f = glm::dot(m_direction, yAxis);

        if (fabs(f) > 0.001f)
        {

            float t1 = (e + AABBMin.y) / f;
            float t2 = (e + AABBMax.y) / f;

            if (t1 > t2)
            {
                float w = t1;
                t1 = t2;
                t2 = w;
            }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;
        }
        else
        {
            if (-e + AABBMin.y > 0.0f || -e + AABBMax.y < 0.0f)
                return false;
        }
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Z axis
    // Exactly the same thing than above.
    {
        glm::vec3 zAxis(transform[2].x, transform[2].y, transform[2].z);
        float e = glm::dot(zAxis, delta);
        float f = glm::dot(m_direction, zAxis);

        if (fabs(f) > 0.001f)
        {

            float t1 = (e + AABBMin.z) / f;
            float t2 = (e + AABBMax.z) / f;

            if (t1 > t2)
            {
                float w = t1;
                t1 = t2;
                t2 = w;
            }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;
        }
        else
        {
            if (-e + AABBMin.z > 0.0f || -e + AABBMax.z < 0.0f)
                return false;
        }
    }
    return true;
}

glm::vec3 Ray::GetEnd() const
{
    return m_start + m_direction * m_length;
}

std::string FileUtils::LoadFileAsString(const std::filesystem::path &path)
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

void FileUtils::OpenFile(
    const std::string &dialogTitle,
    const std::string &fileType,
    const std::vector<std::string> &extensions,
    const std::function<void(const std::filesystem::path &path)> &func,
    bool projectDirCheck)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    if (ImGui::Button(dialogTitle.c_str()))
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)Windows::GetWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        std::string filters = fileType + " (";
        for (int i = 0; i < extensions.size(); i++)
        {
            filters += "*" + extensions[i];
            if (i < extensions.size() - 1)
                filters += ", ";
        }
        filters += ") ";
        std::string filters2;
        for (int i = 0; i < extensions.size(); i++)
        {
            filters2 += "*" + extensions[i];
            if (i < extensions.size() - 1)
                filters2 += ";";
        }
        char actualFilter[256];
        char title[256];
        strcpy(title, dialogTitle.c_str());
        int index = 0;
        for (auto &i : filters)
        {
            actualFilter[index] = i;
            index++;
        }
        actualFilter[index] = 0;
        index++;
        for (auto &i : filters2)
        {
            actualFilter[index] = i;
            index++;
        }
        actualFilter[index] = 0;
        index++;
        actualFilter[index] = 0;
        index++;
        ofn.lpstrFilter = actualFilter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrTitle = title;
        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            std::string retVal = ofn.lpstrFile;
            const std::string search = "\\";
            size_t pos = retVal.find(search);
            // Repeat till end is reached
            while (pos != std::string::npos)
            {
                // Replace this occurrence of Sub String
                retVal.replace(pos, 1, "/");
                // Get the next occurrence from the current position
                pos = retVal.find(search, pos + 1);
            }
            std::filesystem::path path = retVal;
            if (!projectDirCheck || ProjectManager::IsInProjectFolder(path))
                func(path);
        }
    }
#else
    if (ImGui::Button(dialogTitle.c_str()))
        ImGui::OpenPopup(dialogTitle.c_str());
    static imgui_addons::ImGuiFileBrowser file_dialog;
    std::string filters;
    for (int i = 0; i < extensions.size(); i++)
    {
        filters += extensions[i];
        if (i < extensions.size() - 1)
            filters += ",";
    }
    if (file_dialog.showFileDialog(
            dialogTitle, imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), filters))
    {
        func(file_dialog.selected_path);
    }
#endif
}

std::pair<bool, uint32_t> FileUtils::DirectoryTreeViewRecursive(
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
void FileUtils::SaveFile(
    const std::string &dialogTitle,
    const std::string &fileType,
    const std::vector<std::string> &extensions,
    const std::function<void(const std::filesystem::path &)> &func,
    bool projectDirCheck)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    if (ImGui::Button(dialogTitle.c_str()))
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)Windows::GetWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        std::string filters = fileType + " (";
        for (int i = 0; i < extensions.size(); i++)
        {
            filters += "*" + extensions[i];
            if (i < extensions.size() - 1)
                filters += ", ";
        }
        filters += ") ";
        std::string filters2;
        for (int i = 0; i < extensions.size(); i++)
        {
            filters2 += "*" + extensions[i];
            if (i < extensions.size() - 1)
                filters2 += ";";
        }
        char actualFilter[256];
        char title[256];
        strcpy(title, dialogTitle.c_str());
        int index = 0;
        for (auto &i : filters)
        {
            actualFilter[index] = i;
            index++;
        }
        actualFilter[index] = 0;
        index++;
        for (auto &i : filters2)
        {
            actualFilter[index] = i;
            index++;
        }
        actualFilter[index] = 0;
        index++;
        actualFilter[index] = 0;
        index++;
        ofn.lpstrFilter = actualFilter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrTitle = title;
        // Sets the default extension by extracting it from the filter
        ofn.lpstrDefExt = strchr(actualFilter, '\0') + 1;

        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            std::string retVal = ofn.lpstrFile;
            const std::string search = "\\";
            size_t pos = retVal.find(search);
            // Repeat till end is reached
            while (pos != std::string::npos)
            {
                // Replace this occurrence of Sub String
                retVal.replace(pos, 1, "/");
                // Get the next occurrence from the current position
                pos = retVal.find(search, pos + 1);
            }
            std::filesystem::path path = retVal;
            if (!projectDirCheck || ProjectManager::IsInProjectFolder(path))
                func(path);
        }
    }
#else
    if (ImGui::Button(dialogTitle.c_str()))
        ImGui::OpenPopup(dialogTitle.c_str());
    static imgui_addons::ImGuiFileBrowser file_dialog;
    std::string filters;
    for (int i = 0; i < extensions.size(); i++)
    {
        filters += extensions[i];
        if (i < extensions.size() - 1)
            filters += ",";
    }
    if (file_dialog.showFileDialog(
            dialogTitle, imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), filters))
    {
        std::filesystem::path path = file_dialog.selected_path;
        func(path);
    }
#endif
}
void FileUtils::OpenFolder(
    const std::string &dialogTitle,
    const std::function<void(const std::filesystem::path &path)> &func,
    bool projectDirCheck)
{
    if (ImGui::Button(dialogTitle.c_str()))
    {
        TCHAR path[MAX_PATH];
        BROWSEINFO bi = {0};
        bi.lpszTitle = dialogTitle.c_str();
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        //bi.lpfn       = BrowseCallbackProc;
        //bi.lParam     = (LPARAM) path_param;
        LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
        if (pidl != nullptr)
        {
            // get the name of the folder and put it in path
            SHGetPathFromIDList(pidl, path);
            // free memory used
            IMalloc *imalloc = nullptr;
            if (SUCCEEDED(SHGetMalloc(&imalloc)))
            {
                imalloc->Free(pidl);
                imalloc->Release();
            }
            std::string retVal = path;
            const std::string search = "\\";
            size_t pos = retVal.find(search);
            // Repeat till end is reached
            while (pos != std::string::npos)
            {
                // Replace this occurrence of Sub String
                retVal.replace(pos, 1, "/");
                // Get the next occurrence from the current position
                pos = retVal.find(search, pos + 1);
            }
            std::filesystem::path path = retVal;
            if (!projectDirCheck || ProjectManager::IsInProjectFolder(path))
                func(path);
        }
    }
}


bool ImGui::Splitter(
    bool split_vertically,
    float thickness,
    float &size1,
    float &size2,
    float min_size1,
    float min_size2,
    float splitter_long_axis_size)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(size1, 0.0f) : ImVec2(0.0f, size1));
    bb.Max = bb.Min + CalcItemSize(
                          split_vertically ? ImVec2(thickness, splitter_long_axis_size)
                                           : ImVec2(splitter_long_axis_size, thickness),
                          0.0f,
                          0.0f);
    return SplitterBehavior(
        bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, &size1, &size2, min_size1, min_size2, 0.0f);
}

bool ImGui::Combo(const std::string& label, const std::vector<std::string>& items, unsigned& currentSelection, ImGuiComboFlags flags)
{
    bool modified = false;
	currentSelection = glm::clamp(currentSelection, 0u, static_cast<unsigned>(items.size()));
    if (ImGui::BeginCombo(
        label.c_str(),
        items[currentSelection]
        .c_str(), flags)) // The second parameter is the label previewed before opening the combo.
    {
        for(unsigned i = 0; i < items.size(); i++)
        {
            const bool selected = currentSelection == i;
            if (ImGui::Selectable(items[i].c_str(), selected))
            {
                currentSelection = i;
                modified = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();           // You may set the initial focus when opening the combo (scrolling
                                                      // + for keyboard navigation support)
            }
        }
        ImGui::EndCombo();
    }
    return modified;
}

bool ImGui::Combo(const std::string& label, const std::vector<std::string>& items, int& currentSelection, ImGuiComboFlags flags)
{
    bool modified = false;
    currentSelection = glm::clamp(currentSelection, 0, static_cast<int>(items.size()));
    if (ImGui::BeginCombo(
        label.c_str(),
        items[currentSelection]
        .c_str(), flags)) // The second parameter is the label previewed before opening the combo.
    {
        for (int i = 0; i < items.size(); i++)
        {
            const bool selected = currentSelection == i;
            if (ImGui::Selectable(items[i].c_str(), selected))
            {
                currentSelection = i;
                modified = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();           // You may set the initial focus when opening the combo (scrolling
                // + for keyboard navigation support)
            }
        }
        ImGui::EndCombo();
    }
    return modified;
}

void SphereMeshGenerator::Icosahedron(std::vector<glm::vec3> &vertices, std::vector<glm::uvec3> &triangles)
{
    vertices.clear();
    triangles.clear();

    float phi = (1.0f + glm::sqrt(5.0f)) * 0.5f; // golden ratio
    float a = 1.0f;
    float b = 1.0f / phi;

    // add vertices
    vertices.push_back(glm::normalize(glm::vec3(0, b, -a)));
    vertices.push_back(glm::normalize(glm::vec3(b, a, 0)));
    vertices.push_back(glm::normalize(glm::vec3(-b, a, 0)));
    vertices.push_back(glm::normalize(glm::vec3(0, b, a)));
    vertices.push_back(glm::normalize(glm::vec3(0, -b, a)));
    vertices.push_back(glm::normalize(glm::vec3(-a, 0, b)));
    vertices.push_back(glm::normalize(glm::vec3(0, -b, -a)));
    vertices.push_back(glm::normalize(glm::vec3(a, 0, -b)));
    vertices.push_back(glm::normalize(glm::vec3(a, 0, b)));
    vertices.push_back(glm::normalize(glm::vec3(-a, 0, -b)));
    vertices.push_back(glm::normalize(glm::vec3(b, -a, 0)));
    vertices.push_back(glm::normalize(glm::vec3(-b, -a, 0)));

    // add triangles
    triangles.push_back(glm::uvec3(3, 2, 1));
    triangles.push_back(glm::uvec3(2, 3, 4));
    triangles.push_back(glm::uvec3(6, 5, 4));
    triangles.push_back(glm::uvec3(5, 9, 4));
    triangles.push_back(glm::uvec3(8, 7, 1));
    triangles.push_back(glm::uvec3(7, 10, 1));
    triangles.push_back(glm::uvec3(12, 11, 5));
    triangles.push_back(glm::uvec3(11, 12, 7));
    triangles.push_back(glm::uvec3(10, 6, 3));
    triangles.push_back(glm::uvec3(6, 10, 12));
    triangles.push_back(glm::uvec3(9, 8, 2));
    triangles.push_back(glm::uvec3(8, 9, 11));
    triangles.push_back(glm::uvec3(3, 6, 4));
    triangles.push_back(glm::uvec3(9, 2, 4));
    triangles.push_back(glm::uvec3(10, 3, 1));
    triangles.push_back(glm::uvec3(2, 8, 1));
    triangles.push_back(glm::uvec3(12, 10, 7));
    triangles.push_back(glm::uvec3(8, 11, 7));
    triangles.push_back(glm::uvec3(6, 12, 5));
    triangles.push_back(glm::uvec3(11, 9, 5));

    for (auto &i : triangles)
    {
        i.x -= 1;
        i.y -= 1;
        i.z -= 1;
    }
}
