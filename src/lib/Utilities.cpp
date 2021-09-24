#include <ConsoleManager.hpp>
#include <EntityManager.hpp>
#include <Gui.hpp>
#include <ImGuiFileBrowser.hpp>
#include <ProjectManager.hpp>
#include <Utilities.hpp>
#include <WindowManager.hpp>
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
        const float avail = ImGui::GetContentRegionAvailWidth();
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
    const std::function<void(const std::filesystem::path &path)> &func, bool projectDirCheck)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    if (ImGui::Button(dialogTitle.c_str()))
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)WindowManager::GetWindow());
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
    const std::function<void(const std::filesystem::path &)> &func, bool projectDirCheck)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    if (ImGui::Button(dialogTitle.c_str()))
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)WindowManager::GetWindow());
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
    if (file_dialog.showFileDialog(
            dialogTitle, imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), filters))
    {
        std::filesystem::path path = file_dialog.selected_path;
        func(path);
    }
#endif
}

// TAKEN FROM (with much cleaning + tweaking):
// https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930

int Curve::CurveEditor(const std::string &label, const ImVec2 &editor_size, unsigned int flags)
{
    enum class StorageValues : ImGuiID
    {
        FROM_X = 100,
        FROM_Y,
        WIDTH,
        HEIGHT,
        IS_PANNING,
        POINT_START_X,
        POINT_START_Y
    };

    if (ImGui::TreeNodeEx(label.c_str()))
    {
        bool noTangent = !m_tangent;
        auto &values = m_values;
        if (noTangent && values.size() == 0 || !noTangent && values.size() < 6)
        {
            Clear();
        }
        if (ImGui::Button("Clear"))
            Clear();
        static ImVec2 start_pan;
        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        ImVec2 size = editor_size;

        size.x = size.x < 0 ? ImGui::GetContentRegionAvailWidth() : size.x;
        size.y = size.y < 0 ? size.x / 2.0f : size.y;

        ImGuiWindow *parent_window = ImGui::GetCurrentWindow();
        ImGuiID id = parent_window->GetID(label.c_str());
        if (!ImGui::BeginChildFrame(id, size, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::EndChildFrame();
            ImGui::TreePop();
            return -1;
        }

        int hovered_idx = -1;

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndChildFrame();
            ImGui::TreePop();
            return -1;
        }

        ImVec2 points_min(FLT_MAX, FLT_MAX);
        ImVec2 points_max(-FLT_MAX, -FLT_MAX);

        bool allowRemoveSides = (unsigned)flags & (unsigned)CurveEditorFlags::ALLOW_REMOVE_SIDES;

        int points_count = 0;
        if (noTangent)
        {
            points_count = values.size();
        }
        else
        {
            points_count = values.size() / 3;
        }
        for (int point_idx = 0; point_idx < points_count; ++point_idx)
        {
            ImVec2 point;
            if (noTangent)
            {
                point = ((ImVec2 *)values.data())[point_idx];
            }
            else
            {
                point = ((ImVec2 *)values.data())[1 + point_idx * 3];
            }
            points_max = ImMax(points_max, point);
            points_min = ImMin(points_min, point);
        }
        points_max.y = ImMax(points_max.y, points_min.y + 0.0001f);

        float from_x = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_X, m_min.x);
        float from_y = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_Y, m_min.y);
        float width = window->StateStorage.GetFloat((ImGuiID)StorageValues::WIDTH, m_max.x - m_min.x);
        float height = window->StateStorage.GetFloat((ImGuiID)StorageValues::HEIGHT, m_max.y - m_min.y);
        window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_X, from_x);
        window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_Y, from_y);
        window->StateStorage.SetFloat((ImGuiID)StorageValues::WIDTH, width);
        window->StateStorage.SetFloat((ImGuiID)StorageValues::HEIGHT, height);

        const ImRect inner_bb = window->InnerRect;
        const ImRect frame_bb(inner_bb.Min - style.FramePadding, inner_bb.Max + style.FramePadding);

        auto transform = [&](const ImVec2 &pos) -> ImVec2 {
            float x = (pos.x - from_x) / width;
            float y = (pos.y - from_y) / height;

            return ImVec2(inner_bb.Min.x * (1 - x) + inner_bb.Max.x * x, inner_bb.Min.y * y + inner_bb.Max.y * (1 - y));
        };

        auto invTransform = [&](const ImVec2 &pos) -> ImVec2 {
            float x = (pos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x);
            float y = (inner_bb.Max.y - pos.y) / (inner_bb.Max.y - inner_bb.Min.y);

            return ImVec2(from_x + width * x, from_y + height * y);
        };

        if ((unsigned)flags & (unsigned)CurveEditorFlags::SHOW_GRID)
        {
            int exp;
            frexp(width / 5, &exp);
            float step_x = (float)ldexp(1.0, exp);
            int cell_cols = int(width / step_x);

            float x = step_x * int(from_x / step_x);
            for (int i = -1; i < cell_cols + 2; ++i)
            {
                ImVec2 a = transform({x + i * step_x, from_y});
                ImVec2 b = transform({x + i * step_x, from_y + height});
                window->DrawList->AddLine(a, b, 0x55000000);
                char buf[64];
                if (exp > 0)
                {
                    ImFormatString(buf, sizeof(buf), " %d", int(x + i * step_x));
                }
                else
                {
                    ImFormatString(buf, sizeof(buf), " %f", x + i * step_x);
                }
                window->DrawList->AddText(b, 0x55000000, buf);
            }

            frexp(height / 5, &exp);
            float step_y = (float)ldexp(1.0, exp);
            int cell_rows = int(height / step_y);

            float y = step_y * int(from_y / step_y);
            for (int i = -1; i < cell_rows + 2; ++i)
            {
                ImVec2 a = transform({from_x, y + i * step_y});
                ImVec2 b = transform({from_x + width, y + i * step_y});
                window->DrawList->AddLine(a, b, 0x55000000);
                char buf[64];
                if (exp > 0)
                {
                    ImFormatString(buf, sizeof(buf), " %d", int(y + i * step_y));
                }
                else
                {
                    ImFormatString(buf, sizeof(buf), " %f", y + i * step_y);
                }
                window->DrawList->AddText(a, 0x55000000, buf);
            }
        }

        if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsItemHovered())
        {
            float scale = powf(2, ImGui::GetIO().MouseWheel);
            width *= scale;
            height *= scale;
            window->StateStorage.SetFloat((ImGuiID)StorageValues::WIDTH, width);
            window->StateStorage.SetFloat((ImGuiID)StorageValues::HEIGHT, height);
        }
        if (ImGui::IsMouseReleased(2))
        {
            window->StateStorage.SetBool((ImGuiID)StorageValues::IS_PANNING, false);
        }
        if (window->StateStorage.GetBool((ImGuiID)StorageValues::IS_PANNING, false))
        {
            ImVec2 drag_offset = ImGui::GetMouseDragDelta(2);
            from_x = start_pan.x;
            from_y = start_pan.y;
            from_x -= drag_offset.x * width / (inner_bb.Max.x - inner_bb.Min.x);
            from_y += drag_offset.y * height / (inner_bb.Max.y - inner_bb.Min.y);
            window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_X, from_x);
            window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_Y, from_y);
        }
        else if (ImGui::IsMouseDragging(2) && ImGui::IsItemHovered())
        {
            window->StateStorage.SetBool((ImGuiID)StorageValues::IS_PANNING, true);
            start_pan.x = from_x;
            start_pan.y = from_y;
        }

        int changed_idx = -1;
        for (int point_idx = points_count - 2; point_idx >= 0; --point_idx)
        {
            ImVec2 *points;
            if (noTangent)
            {
                points = ((ImVec2 *)values.data()) + point_idx;
            }
            else
            {
                points = ((ImVec2 *)values.data()) + 1 + point_idx * 3;
            }

            ImVec2 p_prev = points[0];
            ImVec2 tangent_last;
            ImVec2 tangent;
            ImVec2 p;
            if (noTangent)
            {
                p = points[1];
            }
            else
            {
                tangent_last = points[1];
                tangent = points[2];
                p = points[3];
            }
            int *selected_point = 0;
            auto handlePoint = [&](ImVec2 &p, int idx) -> bool {
                float SIZE = size.x / 100.0f;

                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 pos = transform(p);

                ImGui::SetCursorScreenPos(pos - ImVec2(SIZE, SIZE));
                ImGui::PushID(idx);
                ImGui::InvisibleButton("", ImVec2(SIZE * 2, SIZE * 2));

                bool is_selected = selected_point && *selected_point == point_idx + idx;
                float thickness = is_selected ? 2.0f : 1.0f;
                ImU32 col = ImGui::IsItemActive() || ImGui::IsItemHovered()
                                ? ImGui::GetColorU32(ImGuiCol_PlotLinesHovered)
                                : ImGui::GetColorU32(ImGuiCol_PlotLines);

                window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, -SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, -SIZE), col, thickness);

                if (ImGui::IsItemHovered())
                    hovered_idx = point_idx + idx;

                bool changed = false;
                if (ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
                {
                    if (selected_point)
                        *selected_point = point_idx + idx;
                    window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
                    window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
                }

                if (ImGui::IsItemHovered() || ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    char tmp[64];
                    ImFormatString(tmp, sizeof(tmp), "%0.2f, %0.2f", p.x, p.y);
                    window->DrawList->AddText({pos.x, pos.y - ImGui::GetTextLineHeight()}, 0xff000000, tmp);
                }

                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    pos.x = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
                    pos.y = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
                    pos += ImGui::GetMouseDragDelta();
                    ImVec2 v = invTransform(pos);

                    p = v;
                    changed = true;
                }
                ImGui::PopID();

                ImGui::SetCursorScreenPos(cursor_pos);
                return changed;
            };

            auto handleTangent = [&](ImVec2 &t, const ImVec2 &p, int idx) -> bool {
                float SIZE = size.x / 100.0f;

                auto normalized = [](const ImVec2 &v) -> ImVec2 {
                    float len = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
                    return ImVec2(v.x * len, v.y * len);
                };

                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 pos = transform(p);
                ImVec2 tang = pos + ImVec2(t.x, -t.y) * size.x;

                ImGui::SetCursorScreenPos(tang - ImVec2(SIZE, SIZE));
                ImGui::PushID(-idx);
                ImGui::InvisibleButton("", ImVec2(SIZE * 2, SIZE * 2));

                window->DrawList->AddLine(pos, tang, ImGui::GetColorU32(ImGuiCol_PlotLines));

                ImU32 col = ImGui::IsItemHovered() ? ImGui::GetColorU32(ImGuiCol_PlotLinesHovered)
                                                   : ImGui::GetColorU32(ImGuiCol_PlotLines);

                window->DrawList->AddLine(tang + ImVec2(-SIZE, SIZE), tang + ImVec2(SIZE, SIZE), col);
                window->DrawList->AddLine(tang + ImVec2(SIZE, SIZE), tang + ImVec2(SIZE, -SIZE), col);
                window->DrawList->AddLine(tang + ImVec2(SIZE, -SIZE), tang + ImVec2(-SIZE, -SIZE), col);
                window->DrawList->AddLine(tang + ImVec2(-SIZE, -SIZE), tang + ImVec2(-SIZE, SIZE), col);

                bool changed = false;
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    tang = ImGui::GetIO().MousePos - pos;
                    tang = tang / size.x;
                    tang.y *= -1;
                    t = tang;
                    changed = true;
                }
                ImGui::PopID();

                ImGui::SetCursorScreenPos(cursor_pos);
                return changed;
            };

            ImGui::PushID(point_idx);
            if (!noTangent)
            {
                window->DrawList->AddBezierCurve(
                    transform(p_prev),
                    transform(p_prev + tangent_last),
                    transform(p + tangent),
                    transform(p),
                    ImGui::GetColorU32(ImGuiCol_PlotLines),
                    1.0f,
                    20);
                if (handleTangent(tangent_last, p_prev, 0))
                {
                    auto diff = p - p_prev + tangent;
                    points[1] = ImClamp(tangent_last, ImVec2(0, -1), ImVec2(diff.x, 1));
                    changed_idx = point_idx;
                }
                if (handleTangent(tangent, p, 1))
                {
                    auto diff = p - p_prev - tangent_last;
                    points[2] = ImClamp(tangent, ImVec2(-diff.x, -1), ImVec2(0, 1));
                    changed_idx = point_idx + 1;
                }
                if (point_idx < points_count - 2 && handlePoint(p, 1))
                {
                    points[3] = ImClamp(
                        p,
                        ImVec2(p_prev.x + tangent_last.x - tangent.x + 0.001f, -1),
                        ImVec2(points[6].x + points[5].x - points[4].x - 0.001f, 1));
                    changed_idx = point_idx + 1;
                }
            }
            else
            {
                window->DrawList->AddLine(
                    transform(p_prev), transform(p), ImGui::GetColorU32(ImGuiCol_PlotLines), 1.0f);
                if (handlePoint(p, 1))
                {
                    if (p.x <= p_prev.x)
                        p.x = p_prev.x + 0.001f;
                    if (point_idx < points_count - 2 && p.x >= points[2].x)
                    {
                        p.x = points[2].x - 0.001f;
                    }
                    points[1] = p;
                    changed_idx = point_idx + 1;
                }
            }
            ImGui::PopID();
        }

        ImGui::SetCursorScreenPos(inner_bb.Min);

        ImGui::InvisibleButton("bg", inner_bb.Max - inner_bb.Min);
        bool allowResize = (unsigned)flags & (unsigned)CurveEditorFlags::ALLOW_RESIZE;
        if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0) && allowResize)
        {
            ImVec2 mp = ImGui::GetMousePos();
            ImVec2 new_p = invTransform(mp);
            if (!noTangent)
            {
                bool suitable = false;
                for (int i = 0; i < points_count - 1; i++)
                {
                    auto &prev = values[i * 3 + 1];
                    auto &lastT = values[i * 3 + 2];
                    auto &nextT = values[i * 3 + 3];
                    auto &next = values[i * 3 + 4];

                    if (new_p.x - 0.001 > prev.x + lastT.x && new_p.x + 0.001 < next.x + nextT.x)
                    {
                        suitable = true;
                        break;
                    }
                }
                if (suitable)
                {
                    values.resize(values.size() + 3);
                    values[points_count * 3 + 0] = glm::vec2(-0.1f, 0);
                    values[points_count * 3 + 1] = glm::vec2(new_p.x, new_p.y);
                    values[points_count * 3 + 2] = glm::vec2(0.1f, 0);
                    auto compare = [](const void *a, const void *b) -> int {
                        float fa = (((const ImVec2 *)a) + 1)->x;
                        float fb = (((const ImVec2 *)b) + 1)->x;
                        return fa < fb ? -1 : (fa > fb) ? 1 : 0;
                    };
                    qsort(values.data(), points_count + 1, sizeof(ImVec2) * 3, compare);
                    for (int i = 0; i < points_count + 1; i++)
                    {
                        if (values[i * 3 + 1].x != new_p.x)
                            continue;
                        if (i > 0)
                        {
                            values[i * 3].x = glm::clamp(
                                values[i * 3].x,
                                (values[i * 3 - 2].x + values[i * 3 - 1].x) - values[i * 3 + 1].x,
                                0.0f);
                        }
                        if (i < points_count)
                        {
                            values[i * 3 + 2].x = glm::clamp(
                                values[i * 3 + 2].x,
                                0.0f,
                                (values[i * 3 + 4].x + values[i * 3 + 3].x) - values[i * 3 + 1].x);
                        }
                    }
                }
            }
            else
            {
                values.resize(values.size() + 1);
                values[points_count] = glm::vec2(new_p.x, new_p.y);

                auto compare = [](const void *a, const void *b) -> int {
                    float fa = ((const ImVec2 *)a)->x;
                    float fb = ((const ImVec2 *)b)->x;
                    return fa < fb ? -1 : (fa > fb) ? 1 : 0;
                };

                qsort(values.data(), points_count + 1, sizeof(ImVec2), compare);
            }
        }
        if (hovered_idx >= 0 && ImGui::IsMouseDoubleClicked(0) && allowResize && points_count > 2)
        {
            if (allowRemoveSides || (hovered_idx > 0 && hovered_idx < points_count - 1))
            {
                ImVec2 *points = (ImVec2 *)values.data();
                if (!noTangent)
                {
                    for (int j = hovered_idx * 3; j < points_count * 3 - 3; j += 3)
                    {
                        points[j + 0] = points[j + 3];
                        points[j + 1] = points[j + 4];
                        points[j + 2] = points[j + 5];
                    }
                    values.resize(values.size() - 3);
                }
                else
                {
                    for (int j = hovered_idx; j < points_count - 1; ++j)
                    {
                        points[j] = points[j + 1];
                    }
                    values.resize(values.size() - 1);
                }
            }
        }

        ImGui::EndChildFrame();
        if (noTangent)
        {
            ImGui::SliderFloat("L", &values.front().y, m_min.y, m_max.y);
            ImGui::SliderFloat("R", &values.back().y, m_min.y, m_max.y);
        }
        else
        {
            ImGui::SliderFloat("L", &values[1].y, m_min.y, m_max.y);
            ImGui::SliderFloat("R", &values[values.size() - 2].y, m_min.y, m_max.y);
        }
        ImGui::TreePop();
        return changed_idx;
    }
    return -1;
}
std::vector<glm::vec2> &Curve::UnsafeGetValues()
{
    return m_values;
}
void Curve::SetTangent(bool value)
{
    m_tangent = value;
    Clear();
}
bool Curve::IsTangent()
{
    return m_tangent;
}
float Curve::GetValue(float x)
{
    if (m_tangent)
    {
        int pointSize = m_values.size() / 3;
        for (int i = 0; i < pointSize - 1; i++)
        {
            auto &prev = m_values[i * 3 + 1];
            auto &next = m_values[i * 3 + 4];
            if (x >= prev.x && x < next.x)
            {
                float t = (x - prev.x) / (next.x - prev.x);
                float t1 = 1.0 - t;
                return t1 * t1 * t1 * prev.y + 3.0f * t1 * t1 * t * (prev.y + m_values[i * 3 + 2].y) +
                       3.0f * t1 * t * t * (prev.y + m_values[i * 3 + 3].y) + t * t * t * next.y;
            }
        }
        return m_values[m_values.size() - 2].y;
    }
    else
    {
        for (int i = 0; i < m_values.size() - 1; i++)
        {
            auto &prev = m_values[i];
            auto &next = m_values[i + 1];
            if (x >= prev.x && x < next.x)
            {
                return prev.y + (next.y - prev.y) * (x - prev.x) / (next.x - prev.x);
            }
        }
        return m_values[m_values.size() - 1].y;
    }
}
Curve::Curve(const glm::vec2 &min, const glm::vec2 &max, bool tangent)
{
    m_tangent = tangent;
    m_min = min;
    m_max = max;
    Clear();
}
Curve::Curve(float start, float end, const glm::vec2 &min, const glm::vec2 &max, bool tangent)
{
    m_min = min;
    m_max = max;
    start = glm::clamp(start, m_min.y, m_max.y);
    end = glm::clamp(end, m_min.y, m_max.y);
    m_tangent = tangent;
    if (!m_tangent)
    {
        m_values.clear();
        m_values.push_back({m_min.x, start});
        m_values.push_back({m_max.x, end});
    }
    else
    {
        m_values.clear();
        m_values.push_back({-(m_max.y - m_min.y) / 10.0f, 0.0f});
        m_values.push_back({m_min.x, start});
        m_values.push_back({(m_max.y - m_min.y) / 10.0f, 0.0f});

        m_values.push_back({-(m_max.y - m_min.y) / 10.0f, 0.0f});
        m_values.push_back({m_max.x, end});
        m_values.push_back({(m_max.y - m_min.y) / 10.0f, 0.0f});
    }
}
void Curve::SetStart(float value)
{
    if (!m_tangent)
    {
        m_values.front().y = glm::clamp(value, m_min.y, m_max.y);
    }
    else
    {
        m_values[1].y = glm::clamp(value, m_min.y, m_max.y);
    }
}
void Curve::SetEnd(float value)
{
    if (!m_tangent)
    {
        m_values.back().y = glm::clamp(value, m_min.y, m_max.y);
    }
    else
    {
        m_values[m_values.size() - 2].y = glm::clamp(value, m_min.y, m_max.y);
    }
}
void Curve::Clear()
{
    if (!m_tangent)
    {
        m_values.clear();
        m_values.push_back({m_min.x, (m_min.y + m_max.y) / 2.0f});
        m_values.push_back({m_max.x, (m_min.y + m_max.y) / 2.0f});
    }
    else
    {
        m_values.clear();
        m_values.push_back({-(m_max.y - m_min.y) / 10.0f, 0.0f});
        m_values.push_back({m_min.x, (m_min.y + m_max.y) / 2.0f});
        m_values.push_back({(m_max.y - m_min.y) / 10.0f, 0.0f});

        m_values.push_back({-(m_max.y - m_min.y) / 10.0f, 0.0f});
        m_values.push_back({m_max.x, (m_min.y + m_max.y) / 2.0f});
        m_values.push_back({(m_max.y - m_min.y) / 10.0f, 0.0f});
    }
}
void Curve::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_tangent" << m_tangent;
    out << YAML::Key << "m_min" << m_min;
    out << YAML::Key << "m_max" << m_max;

    if (!m_values.empty())
    {
        out << YAML::Key << "m_values" << YAML::BeginSeq;
        for (auto &i : m_values)
        {
            out << i;
        }
        out << YAML::EndSeq;
    }
}
void Curve::Deserialize(const YAML::Node &in)
{
    m_tangent = in["m_tangent"].as<bool>();
    m_min = in["m_min"].as<glm::vec2>();
    m_max = in["m_max"].as<glm::vec2>();
    m_values.clear();
    if (in["m_values"])
    {
        for (const auto &i : in["m_values"])
        {
            m_values.push_back(i.as<glm::vec2>());
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
