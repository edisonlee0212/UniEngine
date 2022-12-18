#include "Plot2D.hpp"
#include "Serialization.hpp"
using namespace UniEngine;


// TAKEN FROM (with much cleaning + tweaking):
// https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930

bool Curve2D::OnInspect(const std::string& label, const ImVec2& editorSize, unsigned flags)
{
    enum class StorageValues : ImGuiID
    {
        FromX = 100,
        FromY,
        Width,
        Height,
        IsPanning,
        PointStartX,
        PointStartY
    };
    int changed_idx = -1;
    bool changed = false;

    if (ImGui::TreeNodeEx(label.c_str()))
    {
        bool noTangent = !m_tangent;
        auto& values = m_values;
        if (noTangent && values.empty() || !noTangent && values.size() < 6)
        {
            Clear();
        }
        if (ImGui::Button("Clear"))
        {
            changed = true;
            Clear();
        }
        static ImVec2 startPan;
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        ImVec2 size = editorSize;

        size.x = size.x < 0 ? ImGui::GetWindowContentRegionWidth() : size.x;
        size.y = size.y < 0 ? size.x / 2.0f : size.y;

        ImGuiWindow* parentWindow = ImGui::GetCurrentWindow();
        if (ImGuiID id = parentWindow->GetID(label.c_str()); !ImGui::BeginChildFrame(id, size, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::EndChildFrame();
            ImGui::TreePop();
            return false;
        }

        int hoveredIdx = -1;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndChildFrame();
            ImGui::TreePop();
            return false;
        }

        ImVec2 pointsMin(FLT_MAX, FLT_MAX);
        ImVec2 pointsMax(-FLT_MAX, -FLT_MAX);

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
        for (int pointIdx = 0; pointIdx < points_count; ++pointIdx)
        {
            ImVec2 point;
            if (noTangent)
            {
                point = reinterpret_cast<ImVec2*>(values.data())[pointIdx];
            }
            else
            {
                point = reinterpret_cast<ImVec2*>(values.data())[1 + pointIdx * 3];
            }
            pointsMax = ImMax(pointsMax, point);
            pointsMin = ImMin(pointsMin, point);
        }
        pointsMax.y = ImMax(pointsMax.y, pointsMin.y + 0.0001f);

        float fromX = window->StateStorage.GetFloat(static_cast<ImGuiID>(StorageValues::FromX), m_min.x);
        float from_y = window->StateStorage.GetFloat(static_cast<ImGuiID>(StorageValues::FromY), m_min.y);
        float width = window->StateStorage.GetFloat(static_cast<ImGuiID>(StorageValues::Width), m_max.x - m_min.x);
        float height = window->StateStorage.GetFloat(static_cast<ImGuiID>(StorageValues::Height), m_max.y - m_min.y);
        window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::FromX), fromX);
        window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::FromY), from_y);
        window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::Width), width);
        window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::Height), height);

        const ImRect innerBb = window->InnerRect;
        const ImRect frameBb(innerBb.Min - style.FramePadding, innerBb.Max + style.FramePadding);

        auto transform = [&](const ImVec2& pos) -> ImVec2 {
            const float x = (pos.x - fromX) / width;
            const float y = (pos.y - from_y) / height;

            return { innerBb.Min.x * (1 - x) + innerBb.Max.x * x, innerBb.Min.y * y + innerBb.Max.y * (1 - y) };
        };

        auto invTransform = [&](const ImVec2& pos) -> ImVec2 {
            float x = (pos.x - innerBb.Min.x) / (innerBb.Max.x - innerBb.Min.x);
            float y = (innerBb.Max.y - pos.y) / (innerBb.Max.y - innerBb.Min.y);

            return { fromX + width * x, from_y + height * y };
        };

        if (flags & static_cast<unsigned>(CurveEditorFlags::SHOW_GRID))
        {
            int exp;
            frexp(width / 5, &exp);
            auto stepX = static_cast<float>(ldexp(1.0, exp));
            int cellCols = static_cast<int>(width / stepX);

            float x = stepX * int(fromX / stepX);
            for (int i = -1; i < cellCols + 2; ++i)
            {
                ImVec2 a = transform({ x + i * stepX, from_y });
                ImVec2 b = transform({ x + i * stepX, from_y + height });
                window->DrawList->AddLine(a, b, 0x55000000);
                char buf[64];
                if (exp > 0)
                {
                    ImFormatString(buf, sizeof(buf), " %d", int(x + i * stepX));
                }
                else
                {
                    ImFormatString(buf, sizeof(buf), " %f", x + i * stepX);
                }
                window->DrawList->AddText(b, 0x55000000, buf);
            }

            frexp(height / 5, &exp);
            auto stepY = static_cast<float>(ldexp(1.0, exp));
            int cellRows = static_cast<int>(height / stepY);

            float y = stepY * static_cast<int>(from_y / stepY);
            for (int i = -1; i < cellRows + 2; ++i)
            {
                ImVec2 a = transform({ fromX, y + i * stepY });
                ImVec2 b = transform({ fromX + width, y + i * stepY });
                window->DrawList->AddLine(a, b, 0x55000000);
                char buf[64];
                if (exp > 0)
                {
                    ImFormatString(buf, sizeof(buf), " %d", static_cast<int>(y + i * stepY));
                }
                else
                {
                    ImFormatString(buf, sizeof(buf), " %f", y + i * stepY);
                }
                window->DrawList->AddText(a, 0x55000000, buf);
            }
        }

        if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsItemHovered())
        {
            float scale = powf(2, ImGui::GetIO().MouseWheel);
            width *= scale;
            height *= scale;
            window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::Width), width);
            window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::Height), height);
        }
        if (ImGui::IsMouseReleased(2))
        {
            window->StateStorage.SetBool(static_cast<ImGuiID>(StorageValues::IsPanning), false);
        }
        if (window->StateStorage.GetBool(static_cast<ImGuiID>(StorageValues::IsPanning), false))
        {
            ImVec2 drag_offset = ImGui::GetMouseDragDelta(2);
            fromX = startPan.x;
            from_y = startPan.y;
            fromX -= drag_offset.x * width / (innerBb.Max.x - innerBb.Min.x);
            from_y += drag_offset.y * height / (innerBb.Max.y - innerBb.Min.y);
            window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::FromX), fromX);
            window->StateStorage.SetFloat(static_cast<ImGuiID>(StorageValues::FromY), from_y);
        }
        else if (ImGui::IsMouseDragging(2) && ImGui::IsItemHovered())
        {
            window->StateStorage.SetBool(static_cast<ImGuiID>(StorageValues::IsPanning), true);
            startPan.x = fromX;
            startPan.y = from_y;
        }

        for (int point_idx = points_count - 2; point_idx >= 0; --point_idx)
        {
            ImVec2* points;
            if (noTangent)
            {
                points = reinterpret_cast<ImVec2*>(values.data()) + point_idx;
            }
            else
            {
                points = reinterpret_cast<ImVec2*>(values.data()) + 1 + point_idx * 3;
            }

            ImVec2 pPrev = points[0];
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
            int* selected_point = 0;
            auto handlePoint = [&](ImVec2& p, int idx) -> bool {
                float SIZE = size.x / 100.0f;

                const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 pos = transform(p);

                ImGui::SetCursorScreenPos(pos - ImVec2(SIZE, SIZE));
                ImGui::PushID(idx);
                ImGui::InvisibleButton("", ImVec2(SIZE * 2, SIZE * 2));

                const bool isSelected = selected_point && *selected_point == point_idx + idx;
                const float thickness = isSelected ? 2.0f : 1.0f;
                ImU32 col = ImGui::IsItemActive() || ImGui::IsItemHovered()
                    ? ImGui::GetColorU32(ImGuiCol_PlotLinesHovered)
                    : ImGui::GetColorU32(ImGuiCol_PlotLines);

                window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, -SIZE), col, thickness);
                window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, -SIZE), col, thickness);

                if (ImGui::IsItemHovered())
                    hoveredIdx = point_idx + idx;

                if (ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
                {
                    if (selected_point)
                        *selected_point = point_idx + idx;
                    window->StateStorage.SetFloat((ImGuiID)StorageValues::PointStartX, pos.x);
                    window->StateStorage.SetFloat((ImGuiID)StorageValues::PointStartY, pos.y);
                }

                if (ImGui::IsItemHovered() || ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    char tmp[64];
                    ImFormatString(tmp, sizeof(tmp), "%0.2f, %0.2f", p.x, p.y);
                    window->DrawList->AddText({ pos.x, pos.y - ImGui::GetTextLineHeight() }, 0xff000000, tmp);
                }
                bool valueChanged = false;
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    pos.x = window->StateStorage.GetFloat((ImGuiID)StorageValues::PointStartX, pos.x);
                    pos.y = window->StateStorage.GetFloat((ImGuiID)StorageValues::PointStartY, pos.y);
                    pos += ImGui::GetMouseDragDelta();
                    ImVec2 v = invTransform(pos);

                    p = v;
                    valueChanged = true;
                }
                ImGui::PopID();

                ImGui::SetCursorScreenPos(cursor_pos);
                return valueChanged;
            };

            auto handleTangent = [&](ImVec2& t, const ImVec2& p, int idx) -> bool {
                float SIZE = size.x / 100.0f;

                auto normalized = [](const ImVec2& v) -> ImVec2 {
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
                bool tangentChanged = false;
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    tang = ImGui::GetIO().MousePos - pos;
                    tang = tang / size.x;
                    tang.y *= -1;
                    t = tang;
                    tangentChanged = true;
                }
                ImGui::PopID();

                ImGui::SetCursorScreenPos(cursor_pos);
                return tangentChanged;
            };

            ImGui::PushID(point_idx);
            if (!noTangent)
            {
                window->DrawList->AddBezierCurve(
                    transform(pPrev),
                    transform(pPrev + tangent_last),
                    transform(p + tangent),
                    transform(p),
                    ImGui::GetColorU32(ImGuiCol_PlotLines),
                    1.0f,
                    20);
                if (handleTangent(tangent_last, pPrev, 0))
                {
                    auto diff = p - pPrev + tangent;
                    points[1] = ImClamp(tangent_last, ImVec2(0, -1), ImVec2(diff.x, 1));
                    changed_idx = point_idx;
                }
                if (handleTangent(tangent, p, 1))
                {
                    auto diff = p - pPrev - tangent_last;
                    points[2] = ImClamp(tangent, ImVec2(-diff.x, -1), ImVec2(0, 1));
                    changed_idx = point_idx + 1;
                }
                if (point_idx < points_count - 2 && handlePoint(p, 1))
                {
                    points[3] = ImClamp(
                        p,
                        ImVec2(pPrev.x + tangent_last.x - tangent.x + 0.001f, m_min.y),
                        ImVec2(points[6].x + points[5].x - points[4].x - 0.001f, m_max.y));
                    changed_idx = point_idx + 1;
                }
            }
            else
            {
                window->DrawList->AddLine(
                    transform(pPrev), transform(p), ImGui::GetColorU32(ImGuiCol_PlotLines), 1.0f);
                if (handlePoint(p, 1))
                {
                    if (p.x <= pPrev.x)
                        p.x = pPrev.x + 0.001f;
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

        ImGui::SetCursorScreenPos(innerBb.Min);

        ImGui::InvisibleButton("bg", innerBb.Max - innerBb.Min);
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
                    auto& prev = values[i * 3 + 1];
                    auto& lastT = values[i * 3 + 2];
                    auto& nextT = values[i * 3 + 3];
                    auto& next = values[i * 3 + 4];

                    if (new_p.x - 0.001 > prev.x + lastT.x && new_p.x + 0.001 < next.x + nextT.x)
                    {
                        suitable = true;
                        break;
                    }
                }
                if (suitable)
                {
                    changed = true;
                    values.resize(values.size() + 3);
                    values[points_count * 3 + 0] = glm::vec2(-0.1f, 0);
                    values[points_count * 3 + 1] = glm::vec2(new_p.x, new_p.y);
                    values[points_count * 3 + 2] = glm::vec2(0.1f, 0);
                    auto compare = [](const void* a, const void* b) -> int {
                        float fa = (((const ImVec2*)a) + 1)->x;
                        float fb = (((const ImVec2*)b) + 1)->x;
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
                changed = true;
                values.resize(values.size() + 1);
                values[points_count] = glm::vec2(new_p.x, new_p.y);

                auto compare = [](const void* a, const void* b) -> int {
                    float fa = ((const ImVec2*)a)->x;
                    float fb = ((const ImVec2*)b)->x;
                    return fa < fb ? -1 : (fa > fb) ? 1 : 0;
                };

                qsort(values.data(), points_count + 1, sizeof(ImVec2), compare);
            }
        }
        if (hoveredIdx >= 0 && ImGui::IsMouseDoubleClicked(0) && allowResize && points_count > 2)
        {
            if (allowRemoveSides || (hoveredIdx > 0 && hoveredIdx < points_count - 1))
            {
                changed = true;
                auto* points = (ImVec2*)values.data();
                if (!noTangent)
                {
                    for (int j = hoveredIdx * 3; j < points_count * 3 - 3; j += 3)
                    {
                        points[j + 0] = points[j + 3];
                        points[j + 1] = points[j + 4];
                        points[j + 2] = points[j + 5];
                    }
                    values.resize(values.size() - 3);
                }
                else
                {
                    for (int j = hoveredIdx; j < points_count - 1; ++j)
                    {
                        points[j] = points[j + 1];
                    }
                    values.resize(values.size() - 1);
                }
            }
        }

        ImGui::EndChildFrame();
        if (!((unsigned)flags & (unsigned)CurveEditorFlags::DISABLE_START_END_Y))
        {
            if (noTangent)
            {
                if (ImGui::SliderFloat("Begin Y", &values.front().y, m_min.y, m_max.y))
                {
                    changed = true;
                }
                if (ImGui::SliderFloat("End Y", &values.back().y, m_min.y, m_max.y))
                {
                    changed = true;
                }
            }
            else
            {
                if (ImGui::SliderFloat("Begin Y", &values[1].y, m_min.y, m_max.y))
                {
                    changed = true;
                }
                if (ImGui::SliderFloat("End Y", &values[values.size() - 2].y, m_min.y, m_max.y))
                {
                    changed = true;
                }
            }
        }
        if ((unsigned)flags & (unsigned)CurveEditorFlags::SHOW_DEBUG)
        {
            static float test = 0.5f;
            ImGui::SliderFloat("X", &test, 0.0f, 1.0f);
            ImGui::Text("Y: %.3f", GetValue(test));
        }
        ImGui::TreePop();
    }
    return changed_idx != -1 || changed;
}
std::vector<glm::vec2>& Curve2D::UnsafeGetValues()
{
    return m_values;
}
void Curve2D::SetTangent(bool value)
{
    m_tangent = value;
    Clear();
}
bool Curve2D::IsTangent()
{
    return m_tangent;
}
float Curve2D::GetValue(float x, unsigned iteration) const
{
    x = glm::clamp(x, 0.0f, 1.0f);
    if (m_tangent)
    {
        int pointSize = m_values.size() / 3;
        for (int i = 0; i < pointSize - 1; i++)
        {
            auto& prev = m_values[i * 3 + 1];
            auto& next = m_values[i * 3 + 4];
            if (x == prev.x)
            {
                return prev.y;
            }
            else if (x > prev.x && x < next.x)
            {
                float realX = (x - prev.x) / (next.x - prev.x);
                float upper = 1.0f;
                float lower = 0.0f;
                float tempT = 0.5f;
                for (unsigned iter = 0; iter < iteration; iter++)
                {
                    float tempT1 = 1.0f - tempT;
                    float globalX = tempT1 * tempT1 * tempT1 * prev.x +
                        3.0f * tempT1 * tempT1 * tempT * (prev.x + m_values[i * 3 + 2].x) +
                        3.0f * tempT1 * tempT * tempT * (next.x + m_values[i * 3 + 3].x) +
                        tempT * tempT * tempT * next.x;
                    float testX = (globalX - prev.x) / (next.x - prev.x);
                    if (testX > realX)
                    {
                        upper = tempT;
                        tempT = (tempT + lower) / 2.0f;
                    }
                    else
                    {
                        lower = tempT;
                        tempT = (tempT + upper) / 2.0f;
                    }
                }
                float tempT1 = 1.0f - tempT;
                return tempT1 * tempT1 * tempT1 * prev.y +
                    3.0f * tempT1 * tempT1 * tempT * (prev.y + m_values[i * 3 + 2].y) +
                    3.0f * tempT1 * tempT * tempT * (next.y + m_values[i * 3 + 3].y) +
                    tempT * tempT * tempT * next.y;
            }
        }
        return m_values[m_values.size() - 2].y;
    }
    else
    {
        for (int i = 0; i < m_values.size() - 1; i++)
        {
            auto& prev = m_values[i];
            auto& next = m_values[i + 1];
            if (x >= prev.x && x < next.x)
            {
                return prev.y + (next.y - prev.y) * (x - prev.x) / (next.x - prev.x);
            }
        }
        return m_values[m_values.size() - 1].y;
    }
}
Curve2D::Curve2D(const glm::vec2& min, const glm::vec2& max, bool tangent)
{
    m_tangent = tangent;
    m_min = min;
    m_max = max;
    Clear();
}
Curve2D::Curve2D(float start, float end, const glm::vec2& min, const glm::vec2& max, bool tangent)
{
    m_min = min;
    m_max = max;
    start = glm::clamp(start, m_min.y, m_max.y);
    end = glm::clamp(end, m_min.y, m_max.y);
    m_tangent = tangent;
    if (!m_tangent)
    {
        m_values.clear();
        m_values.push_back({ m_min.x, start });
        m_values.push_back({ m_max.x, end });
    }
    else
    {
        m_values.clear();
        m_values.push_back({ -(m_max.y - m_min.y) / 10.0f, 0.0f });
        m_values.push_back({ m_min.x, start });
        m_values.push_back({ (m_max.y - m_min.y) / 10.0f, 0.0f });

        m_values.push_back({ -(m_max.y - m_min.y) / 10.0f, 0.0f });
        m_values.push_back({ m_max.x, end });
        m_values.push_back({ (m_max.y - m_min.y) / 10.0f, 0.0f });
    }
}
void Curve2D::SetStart(float value)
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
void Curve2D::SetEnd(float value)
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
void Curve2D::Clear()
{
    if (!m_tangent)
    {
        m_values.clear();
        m_values.push_back({ m_min.x, (m_min.y + m_max.y) / 2.0f });
        m_values.push_back({ m_max.x, (m_min.y + m_max.y) / 2.0f });
    }
    else
    {
        m_values.clear();
        m_values.push_back({ -(m_max.y - m_min.y) / 10.0f, 0.0f });
        m_values.push_back({ m_min.x, (m_min.y + m_max.y) / 2.0f });
        m_values.push_back({ (m_max.y - m_min.y) / 10.0f, 0.0f });

        m_values.push_back({ -(m_max.y - m_min.y) / 10.0f, 0.0f });
        m_values.push_back({ m_max.x, (m_min.y + m_max.y) / 2.0f });
        m_values.push_back({ (m_max.y - m_min.y) / 10.0f, 0.0f });
    }
}
void Curve2D::Serialize(YAML::Emitter& out)
{
    out << YAML::Key << "m_tangent" << m_tangent;
    out << YAML::Key << "m_min" << m_min;
    out << YAML::Key << "m_max" << m_max;

    if (!m_values.empty())
    {
        out << YAML::Key << "m_values" << YAML::BeginSeq;
        for (auto& i : m_values)
        {
            out << i;
        }
        out << YAML::EndSeq;
    }
}
void Curve2D::Deserialize(const YAML::Node& in)
{
    m_tangent = in["m_tangent"].as<bool>();
    m_min = in["m_min"].as<glm::vec2>();
    m_max = in["m_max"].as<glm::vec2>();
    m_values.clear();
    if (in["m_values"])
    {
        for (const auto& i : in["m_values"])
        {
            m_values.push_back(i.as<glm::vec2>());
        }
    }
}