//
// Created by lllll on 8/22/2021.
//

#include <PointCloud.hpp>

#include <Tinyply.hpp>
using namespace UniEngine;
using namespace tinyply;
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
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
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
            std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
        if (normals)
            std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
        if (colors)
            std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
        if (texcoords)
            std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
        if (faces)
            std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
        if (tripstrip)
            std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride)
                      << " total indicies (tristrip) " << std::endl;

        // Example One: converting to your own application types
        {
            const size_t numVerticesBytes = vertices->buffer.size_bytes();
            m_points.resize(vertices->count);
            std::memcpy(m_points.data(), vertices->buffer.get(), numVerticesBytes);
            m_compressed = m_points;
            RecalculateBoundingBox();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}

void PointCloud::OnCreate()
{
}
void PointCloud::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<PointCloud>(target);
}
void PointCloud::OnGui()
{
    ImGui::Text(("Original amount: " + std::to_string(m_points.size())).c_str());
    ImGui::Text(("Compressed amount: " + std::to_string(m_compressed.size())).c_str());
    ImGui::DragFloat("Point size", &m_pointSize, 0.01f, 0.01f, 100.0f);
    static float compressScale = 50.0f;
    ImGui::DragFloat("Compress scale", &compressScale, 0.001f, 0.0001f, 10.0f);
    if (ImGui::Button("Compress"))
    {
        Compress(compressScale);
    }

    if (ImGui::Button("Apply compressed"))
    {
        ApplyCompressed();
    }

    if (ImGui::Button("Apply original"))
    {
        ApplyOriginal();
    }

    FileUtils::OpenFile(("Load PLY file##Particles"), "PointCloud", {".ply"}, [&](const std::filesystem::path &filePath) {
        try
        {
            Load(filePath);
            UNIENGINE_LOG("Loaded from " + filePath.string());
        }
        catch (std::exception &e)
        {
            UNIENGINE_ERROR("Failed to load from " + filePath.string());
        }
    });
    FileUtils::SaveFile(("Save Compressed to PLY##Particles"), "PointCloud", {".ply"}, [&](const std::filesystem::path &filePath) {
        try
        {
            Save(filePath);
            UNIENGINE_LOG("Saved to " + filePath.string());
        }
        catch (std::exception &e)
        {
            UNIENGINE_ERROR("Failed to save to " + filePath.string());
        }
    });
}
void PointCloud::ApplyCompressed()
{
    const auto owner = GetOwner();
    auto particleMatrices = owner.GetOrSetPrivateComponent<Particles>().lock()->m_matrices;
    auto &matrices = particleMatrices->m_value;
    matrices.resize(m_compressed.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(m_compressed[i]) * glm::scale(glm::vec3(m_pointSize));
    }
    particleMatrices->Update();
}
void PointCloud::Compress(float resolution)
{
    RecalculateBoundingBox();
    if (resolution == 0)
    {
        UNIENGINE_ERROR("Resolution invalid!");
        return;
    }
    float xMin = m_boundingBox.m_min.x - std::fmod(m_boundingBox.m_min.x, resolution) -
                 (m_boundingBox.m_min.x < 0 ? resolution : 0);
    float yMin = m_boundingBox.m_min.y - std::fmod(m_boundingBox.m_min.y, resolution) -
                 (m_boundingBox.m_min.y < 0 ? resolution : 0);
    float zMin = m_boundingBox.m_min.z - std::fmod(m_boundingBox.m_min.z, resolution) -
                 (m_boundingBox.m_min.z < 0 ? resolution : 0);

    float xMax = m_boundingBox.m_max.x - std::fmod(m_boundingBox.m_max.x, resolution) +
                 (m_boundingBox.m_max.x > 0 ? resolution : 0);
    float yMax = m_boundingBox.m_max.y - std::fmod(m_boundingBox.m_max.y, resolution) +
                 (m_boundingBox.m_max.y > 0 ? resolution : 0);
    float zMax = m_boundingBox.m_max.z - std::fmod(m_boundingBox.m_max.z, resolution) +
                 (m_boundingBox.m_max.z > 0 ? resolution : 0);

    UNIENGINE_LOG(
        "X, Y, Z MIN: [" + std::to_string(xMin) + ", " + std::to_string(yMin) + ", " + std::to_string(zMin) + "]");
    UNIENGINE_LOG(
        "X, Y, Z MAX: [" + std::to_string(xMax) + ", " + std::to_string(yMax) + ", " + std::to_string(zMax) + "]");

    std::vector<int> voxels;
    int rangeX = ((xMax - xMin) / resolution);
    int rangeY = ((yMax - yMin) / resolution);
    int rangeZ = ((zMax - zMin) / resolution);
    int voxelSize = (rangeX + 1) * (rangeY + 1) * (rangeZ + 1);

    if (voxelSize > 1000000000)
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
        int posX = ((i.x - m_boundingBox.m_min.x) / resolution);
        int posY = ((i.y - m_boundingBox.m_min.y) / resolution);
        int posZ = ((i.z - m_boundingBox.m_min.z) / resolution);
        auto index = posX * (rangeY + 1) * (rangeZ + 1) + posY * (rangeZ + 1) + posZ;
        if (index >= voxelSize)
        {
            UNIENGINE_ERROR("Out of range!");
            continue;
        }
        voxels[index]++;
    }
    m_compressed.clear();

    for (int x = 0; x <= rangeX; x++)
    {
        for (int y = 0; y <= rangeY; y++)
        {
            for (int z = 0; z <= rangeZ; z++)
            {
                auto index = x * (rangeY + 1) * (rangeZ + 1) + y * (rangeZ + 1) + z;
                if (voxels[index] != 0)
                {
                    m_compressed.push_back(glm::vec3(x, y, z) * resolution + m_boundingBox.m_min);
                }
            }
        }
    }
}
void PointCloud::RecalculateBoundingBox()
{
    if (m_points.empty())
    {
        m_boundingBox.m_min = glm::vec3(0.0f);
        m_boundingBox.m_max = glm::vec3(0.0f);
        return;
    }
    glm::vec3 minBound = glm::vec3(static_cast<int>(INT_MAX));
    glm::vec3 maxBound = glm::vec3(static_cast<int>(INT_MIN));
    for (const auto &i : m_points)
    {
        minBound = glm::vec3((glm::min)(minBound.x, i.x), (glm::min)(minBound.y, i.y), (glm::min)(minBound.z, i.z));

        maxBound = glm::vec3((glm::max)(maxBound.x, i.x), (glm::max)(maxBound.y, i.y), (glm::max)(maxBound.z, i.z));
    }
    m_boundingBox.m_max = maxBound;
    m_boundingBox.m_min = minBound;
}
void PointCloud::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_pointSize" << m_pointSize;
    if (!m_points.empty())
    {
        out << YAML::Key << "m_points" << YAML::Value
            << YAML::Binary((const unsigned char *)m_points.data(), m_points.size() * sizeof(glm::vec3));
    }
    if (!m_compressed.empty())
    {
        out << YAML::Key << "m_finalOutput" << YAML::Value
            << YAML::Binary((const unsigned char *)m_compressed.data(), m_compressed.size() * sizeof(glm::vec3));
    }
}
void PointCloud::Deserialize(const YAML::Node &in)
{
    m_pointSize = in["m_pointSize"].as<float>();
    if (in["m_points"])
    {
        auto vertexData = in["m_points"].as<YAML::Binary>();
        m_points.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_points.data(), vertexData.data(), vertexData.size());
    }
    if (in["m_finalOutput"])
    {
        auto vertexData = in["m_finalOutput"].as<YAML::Binary>();
        m_compressed.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_compressed.data(), vertexData.data(), vertexData.size());
    }
}
void PointCloud::ApplyOriginal()
{
    const auto owner = GetOwner();
    auto particleMatrices = owner.GetOrSetPrivateComponent<Particles>().lock()->m_matrices;
    auto &matrices = particleMatrices->m_value;
    matrices.resize(m_points.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(m_points[i]) * glm::scale(glm::vec3(m_pointSize));
    }
    particleMatrices->Update();
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

    cube_file.add_properties_to_element(
        "vertex",
        {"x", "y", "z"},
        Type::FLOAT32,
        m_compressed.size(),
        reinterpret_cast<uint8_t *>(m_compressed.data()),
        Type::INVALID,
        0);
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
