//
// Created by lllll on 8/22/2021.
//

#include "PointCloud.hpp"
#include "Particles.hpp"
#include "Material.hpp"
#include "Scene.hpp"
#include "Tinyply.hpp"
#include "ProjectManager.hpp"
#include "DefaultResources.hpp"
#include "Graphics.hpp"
#include "EditorLayer.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;
using namespace tinyply;

AssetRegistration<PointCloud> PointCloudRegistry("PointCloud", {".uepc"});

void PointCloud::Load(const std::filesystem::path &path)
{
    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;

    try
    {
        file_stream.reset(new std::ifstream(path.string(), std::ios::binary));

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + path.string());

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        PlyFile file;
        file.parse_header(*file_stream);

        std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
        for (const auto &c : file.get_comments())
            std::cout << "\t[ply_header] Comment: " << c << std::endl;
        for (const auto &c : file.get_info())
            std::cout << "\t[ply_header] Info: " << c << std::endl;

        for (const auto &e : file.get_elements())
        {
            std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
            for (const auto &p : e.properties)
            {
                std::cout << "\t[ply_header] \tproperty: " << p.name
                          << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                if (p.isList)
                    std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                std::cout << std::endl;
            }
        }

        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers.
        // See examples below on how to marry your own application-specific data structures with this one.
        std::shared_ptr<PlyData> vertices, normals, colors, texcoords, faces, tripstrip;

        // The header information can be used to programmatically extract properties on elements
        // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // like vertex position are hard-coded:
        try
        {
            vertices = file.request_properties_from_element("vertex", {"x", "y", "z"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"r", "g", "b", "a"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            texcoords = file.request_properties_from_element("vertex", {"u", "v"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        try
        {
            faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
        // are specifically in the file, which is unlikely);
        try
        {
            tripstrip = file.request_properties_from_element("tristrips", {"vertex_indices"}, 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }
        file.read(*file_stream);
        if (vertices)
        {
            m_hasPositions = true;
            std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
        }else{
            m_hasPositions = false;
        }
        if (normals)
        {
            std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
            m_hasNormals = true;
        }else m_hasNormals = false;
        if (colors)
        {
            std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
            m_hasColors = true;
        }else{
            m_hasColors = false;
        }
        if(m_hasPositions)
        {
            // Example One: converting to your own application types
            const size_t numVerticesBytes = vertices->buffer.size_bytes();
            if (vertices->t == tinyply::Type::FLOAT64)
            {
                std::vector<glm::dvec3> points;
                points.resize(vertices->count);
                m_points.resize(vertices->count);
                std::memcpy(points.data(), vertices->buffer.get(), numVerticesBytes);
                for (int i = 0; i < vertices->count; i++)
                {
                    m_points[i].x = points[i].x;
                    m_points[i].y = points[i].y;
                    m_points[i].z = points[i].z;
                }
            }
            else if (vertices->t == tinyply::Type::FLOAT32)
            {
                std::vector<glm::vec3> points;
                points.resize(vertices->count);
                m_points.resize(vertices->count);
                std::memcpy(points.data(), vertices->buffer.get(), numVerticesBytes);

                for (int i = 0; i < vertices->count; i++)
                {
                    m_points[i].x = points[i].x;
                    m_points[i].y = points[i].y;
                    m_points[i].z = points[i].z;
                }
            }
        }
        if(m_hasColors)
        {
            const size_t numVerticesBytes = colors->buffer.size_bytes();
            if (colors->t == tinyply::Type::UINT8)
            {
                std::vector<unsigned char> points;
                points.resize(colors->count * 3);
                m_colors.resize(colors->count);
                std::memcpy(points.data(), colors->buffer.get(), numVerticesBytes);
                for (int i = 0; i < colors->count; i++)
                {
                    m_colors[i].x = points[3 * i] / 255.0f;
                    m_colors[i].y = points[3 * i + 1] / 255.0f;
                    m_colors[i].z = points[3 * i + 2] / 255.0f;
                    m_colors[i].w = 1.0f;
                }
            }
            else if (colors->t == tinyply::Type::FLOAT32)
            {
                std::vector<glm::vec3> points;
                points.resize(colors->count);
                m_colors.resize(colors->count);
                std::memcpy(points.data(), colors->buffer.get(), numVerticesBytes);

                for (int i = 0; i < colors->count; i++)
                {
                    m_colors[i].x = points[i].x;
                    m_colors[i].y = points[i].y;
                    m_colors[i].z = points[i].z;
                    m_colors[i].w = 1.0f;
                }
            }
        }
        RecalculateBoundingBox();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}

void PointCloud::OnCreate()
{
}

void PointCloud::OnInspect()
{
    float speed = 0.1f;
    ImGui::Text("Has Colors: %s", (m_hasColors ? "True" : "False"));
    ImGui::Text("Has Positions: %s", (m_hasPositions ? "True" : "False"));
    ImGui::Text("Has Normals: %s", (m_hasNormals ? "True" : "False"));
    ImGui::DragScalarN("Offset", ImGuiDataType_Double, &m_offset.x, 3);
    ImGui::Text(("Original amount: " + std::to_string(m_points.size())).c_str());
    ImGui::DragFloat("Point size", &m_pointSize, 0.01f, 0.01f, 100.0f);
    ImGui::DragFloat("Compress factor", &m_compressFactor, 0.001f, 0.0001f, 10.0f);

    static bool debugRendering = false;
    ImGui::Checkbox("Debug Rendering", &debugRendering);
    if(debugRendering){
        auto editorLayer = Application::GetLayer<EditorLayer>();
        if(editorLayer) DebugRendering(DefaultResources::Primitives::Cube, editorLayer->m_sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation, m_pointSize);
    }

    if (ImGui::Button("Apply compressed"))
    {
        ApplyCompressed();
    }

    if (ImGui::Button("Apply original"))
    {
        ApplyOriginal();
    }

    FileUtils::OpenFile(
        ("Load PLY file##Particles"), "PointCloud", {".ply"}, [&](const std::filesystem::path &filePath) {
            try
            {
                Load(filePath);
                UNIENGINE_LOG("Loaded from " + filePath.string());
            }
            catch (std::exception &e)
            {
                UNIENGINE_ERROR("Failed to load from " + filePath.string());
            }
        }, false);
    FileUtils::SaveFile(
        ("Save Compressed to PLY##Particles"), "PointCloud", {".ply"}, [&](const std::filesystem::path &filePath) {
            try
            {
                Save(filePath);
                UNIENGINE_LOG("Saved to " + filePath.string());
            }
            catch (std::exception &e)
            {
                UNIENGINE_ERROR("Failed to save to " + filePath.string());
            }
        }, false);
    if (ImGui::Button("Clear all points"))
        m_points.clear();
}
void PointCloud::ApplyCompressed()
{
    auto scene = Application::GetActiveScene();
    const auto owner = scene->CreateEntity("Compressed Point Cloud");
    auto particles = scene->GetOrSetPrivateComponent<Particles>(owner).lock();
    particles->m_material = ProjectManager::CreateTemporaryAsset<Material>();
    particles->m_material.Get<Material>()->SetProgram(DefaultResources::GLPrograms::StandardInstancedColoredProgram);
    particles->m_mesh = DefaultResources::Primitives::Cube;
    auto particleMatrices = particles->m_matrices;
    std::vector<glm::mat4> matrices;
    auto compressed = std::vector<glm::dvec3>();
    Compress(compressed);
    matrices.resize(compressed.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate((glm::vec3)(compressed[i] + m_offset)) * glm::scale(glm::vec3(m_pointSize));
    }
    particleMatrices->SetValue(m_colors, matrices);
    particleMatrices->Update();
}
void PointCloud::Compress(std::vector<glm::dvec3> &points)
{
    RecalculateBoundingBox();
    if (m_compressFactor == 0)
    {
        UNIENGINE_ERROR("Resolution invalid!");
        return;
    }
    points.clear();

    double xMin = m_min.x - std::fmod(m_min.x, m_compressFactor) - (m_min.x < 0 ? m_compressFactor : 0);
    double yMin = m_min.y - std::fmod(m_min.y, m_compressFactor) - (m_min.y < 0 ? m_compressFactor : 0);
    double zMin = m_min.z - std::fmod(m_min.z, m_compressFactor) - (m_min.z < 0 ? m_compressFactor : 0);

    double xMax = m_max.x - std::fmod(m_max.x, m_compressFactor) + (m_max.x > 0 ? m_compressFactor : 0);
    double yMax = m_max.y - std::fmod(m_max.y, m_compressFactor) + (m_max.y > 0 ? m_compressFactor : 0);
    double zMax = m_max.z - std::fmod(m_max.z, m_compressFactor) + (m_max.z > 0 ? m_compressFactor : 0);

    UNIENGINE_LOG(
        "X, Y, Z MIN: [" + std::to_string(xMin) + ", " + std::to_string(yMin) + ", " + std::to_string(zMin) + "]");
    UNIENGINE_LOG(
        "X, Y, Z MAX: [" + std::to_string(xMax) + ", " + std::to_string(yMax) + ", " + std::to_string(zMax) + "]");

    std::vector<int> voxels;
    int rangeX = (int)(((xMax - xMin) / (double)m_compressFactor));
    int rangeY = (int)(((yMax - yMin) / (double)m_compressFactor));
    int rangeZ = (int)(((zMax - zMin) / (double)m_compressFactor));
    int voxelSize = (rangeX + 1) * (rangeY + 1) * (rangeZ + 1);

    if (voxelSize > 1000000000 || voxelSize < 0)
    {
        UNIENGINE_ERROR("Resolution too small: " + std::to_string(voxelSize));
        return;
    }
    else
    {
        UNIENGINE_LOG("Voxel size: " + std::to_string(voxelSize));
    }

    voxels.resize(voxelSize);
    memset(voxels.data(), 0, voxelSize * sizeof(int));

    for (const auto &i : m_points)
    {
        int posX = (int)(((i.x - m_min.x) / (double)m_compressFactor));
        int posY = (int)(((i.y - m_min.y) / (double)m_compressFactor));
        int posZ = (int)(((i.z - m_min.z) / (double)m_compressFactor));
        auto index = posX * (rangeY + 1) * (rangeZ + 1) + posY * (rangeZ + 1) + posZ;
        if (index >= voxelSize)
        {
            UNIENGINE_ERROR("Out of range!");
            continue;
        }
        voxels[index]++;
    }

    for (int x = 0; x <= rangeX; x++)
    {
        for (int y = 0; y <= rangeY; y++)
        {
            for (int z = 0; z <= rangeZ; z++)
            {
                auto index = x * (rangeY + 1) * (rangeZ + 1) + y * (rangeZ + 1) + z;
                if (voxels[index] != 0)
                {
                    points.push_back(((glm::dvec3(x, y, z)) * (double)m_compressFactor) + m_min);
                }
            }
        }
    }
}
void PointCloud::RecalculateBoundingBox()
{
    if (m_points.empty())
    {
        m_min = glm::vec3(0.0f);
        m_max = glm::vec3(0.0f);
        return;
    }
    auto minBound = glm::dvec3(static_cast<int>(INT_MAX));
    auto maxBound = glm::dvec3(static_cast<int>(INT_MIN));
    for (const auto &i : m_points)
    {
        minBound = glm::vec3((glm::min)(minBound.x, i.x), (glm::min)(minBound.y, i.y), (glm::min)(minBound.z, i.z));
        maxBound = glm::vec3((glm::max)(maxBound.x, i.x), (glm::max)(maxBound.y, i.y), (glm::max)(maxBound.z, i.z));
    }
    m_max = maxBound;
    m_min = minBound;

    auto avg = glm::dvec3(0);
    for (const auto &i : m_points)
    {
        avg += i / (double)m_points.size();
    }
    m_offset = -m_min;
}
void PointCloud::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_offset" << m_offset;
    out << YAML::Key << "m_pointSize" << m_pointSize;
    out << YAML::Key << "m_compressFactor" << m_compressFactor;
    out << YAML::Key << "m_min" << m_min;
    out << YAML::Key << "m_max" << m_max;
    if (!m_points.empty())
    {
        out << YAML::Key << "m_scatteredPoints" << YAML::Value
            << YAML::Binary((const unsigned char *)m_points.data(), m_points.size() * sizeof(glm::dvec3));
    }
    if (!m_normals.empty())
    {
        out << YAML::Key << "m_normals" << YAML::Value
            << YAML::Binary((const unsigned char *)m_normals.data(), m_normals.size() * sizeof(glm::dvec3));
    }
    if (!m_colors.empty())
    {
        out << YAML::Key << "m_colors" << YAML::Value
            << YAML::Binary((const unsigned char *)m_colors.data(), m_colors.size() * sizeof(glm::vec3));
    }
}
void PointCloud::Deserialize(const YAML::Node &in)
{
    if (in["m_offset"])
        m_offset = in["m_offset"].as<glm::dvec3>();
    m_pointSize = in["m_pointSize"].as<float>();
    m_compressFactor = in["m_compressFactor"].as<float>();
    if (in["m_min"])
        m_min = in["m_min"].as<glm::dvec3>();
    if (in["m_max"])
        m_max = in["m_max"].as<glm::dvec3>();
    if (in["m_scatteredPoints"])
    {
        m_hasPositions = true;
        auto vertexData = in["m_scatteredPoints"].as<YAML::Binary>();
        m_points.resize(vertexData.size() / sizeof(glm::dvec3));
        std::memcpy(m_points.data(), vertexData.data(), vertexData.size());
    }else{
        m_hasPositions = false;
    }

    if (in["m_colors"])
    {
        m_hasColors = true;
        auto vertexData = in["m_colors"].as<YAML::Binary>();
        m_colors.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_colors.data(), vertexData.data(), vertexData.size());
    }else{
        m_hasColors = false;
    }
}
void PointCloud::ApplyOriginal()
{
    auto scene = Application::GetActiveScene();
    const auto owner = scene->CreateEntity("Original Point Cloud");
    auto particles = scene->GetOrSetPrivateComponent<Particles>(owner).lock();
    particles->m_material = ProjectManager::CreateTemporaryAsset<Material>();
    particles->m_material.Get<Material>()->SetProgram(DefaultResources::GLPrograms::StandardInstancedColoredProgram);
    particles->m_mesh = DefaultResources::Primitives::Cube;
    auto particleMatrices = particles->m_matrices;
    std::vector<glm::mat4> matrices;
    matrices.resize(m_points.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(glm::vec3(m_points[i] + m_offset)) * glm::scale(glm::vec3(m_pointSize));
    }
    particleMatrices->SetValue(m_colors, matrices);
    particleMatrices->Update();
}

void PointCloud::DebugRendering(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Camera> &camera, const glm::vec3 &cameraPosition,
                                const glm::quat &cameraRotation, float pointSize) const
{
    std::vector<glm::mat4> matrices;
    matrices.resize(m_points.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(glm::vec3(m_points[i] + m_offset)) * glm::scale(glm::vec3(m_pointSize));

    }
    if(m_hasColors) Gizmos::DrawGizmoMeshInstancedColored(mesh, camera, cameraPosition, cameraRotation, m_colors, matrices, glm::mat4(1.0f), pointSize);
    else Gizmos::DrawGizmoMeshInstanced(mesh, camera, cameraPosition, cameraRotation, glm::vec4(1.0f, 1.0f, 1.0f, 0.8f), matrices, glm::mat4(1.0f), pointSize);
}

void PointCloud::Save(const std::filesystem::path &path)
{
    std::filebuf fb_binary;
    fb_binary.open(path.string(), std::ios::out | std::ios::binary);
    std::ostream outstream_binary(&fb_binary);
    if (outstream_binary.fail())
        throw std::runtime_error("failed to open " + path.string());
    /*
    std::filebuf fb_ascii;
    fb_ascii.open(filename + "-ascii.ply", std::ios::out);
    std::ostream outstream_ascii(&fb_ascii);
    if (outstream_ascii.fail()) throw std::runtime_error("failed to open " + filename);
    */
    PlyFile cube_file;
    if(m_hasPositions)
    {
        cube_file.add_properties_to_element(
            "vertex",
            {"x", "z", "y"},
            Type::FLOAT64,
            m_points.size(),
            reinterpret_cast<uint8_t *>(m_points.data()),
            Type::INVALID,
            0);
    }
    if(m_hasColors){
        cube_file.add_properties_to_element(
            "vertex",
            {"red", "green", "blue"},
            Type::FLOAT32,
            m_colors.size(),
            reinterpret_cast<uint8_t *>(m_colors.data()),
            Type::INVALID,
            0);
    }
    /*
    cube_file.add_properties_to_element("vertex", { "nx", "ny", "nz" },
                                        Type::FLOAT32, cube.normals.size(),
    reinterpret_cast<uint8_t*>(cube.normals.data()), Type::INVALID, 0);

    cube_file.add_properties_to_element("vertex", { "u", "v" },
                                        Type::FLOAT32, cube.texcoords.size() ,
    reinterpret_cast<uint8_t*>(cube.texcoords.data()), Type::INVALID, 0);

    cube_file.add_properties_to_element("face", { "vertex_indices" },
                                        Type::UINT32, cube.triangles.size(),
    reinterpret_cast<uint8_t*>(cube.triangles.data()), Type::UINT8, 3);

    cube_file.get_comments().push_back("generated by tinyply 2.3");

    // Write an ASCII file
    cube_file.write(outstream_ascii, false);
    */
    // Write a binary file
    cube_file.write(outstream_binary, true);
}
void PointCloud::Crop(std::vector<glm::dvec3>& points, const glm::dvec3& min, const glm::dvec3& max)
{
    for(const auto& i : m_points){
        if(i.x >= min.x && i.y >= min.y && i.z >= min.z && i.x <= max.x && i.y <= max.y && i.z <= max.z){
            points.push_back(i);
        }
    }
}

