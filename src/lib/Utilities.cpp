#include <EntityManager.hpp>
#include <Gui.hpp>
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
BezierCubic2D::BezierCubic2D()
{
    m_controlPoints[0] = glm::vec2(0);
    m_controlPoints[1] = glm::vec2(0.5f, 0.0f);
    m_controlPoints[2] = glm::vec2(0.5f, 1.0f);
    m_controlPoints[3] = glm::vec2(1, 1);
}
glm::vec2 BezierCubic2D::GetPoint(const float &t) const
{
    float t1 = 1.0f - t;

    return t1 * t1 * t1 * m_controlPoints[0] + 3.0f * t1 * t1 * t * m_controlPoints[1] +
           3.0f * t1 * t * t * m_controlPoints[2] + t * t * t * m_controlPoints[3];
}
bool BezierCubic2D::Graph(const std::string &label)
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
            return changed;

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
