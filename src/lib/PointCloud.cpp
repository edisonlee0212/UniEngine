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
        const size_t numVerticesBytes = vertices->buffer.size_bytes();
        if(vertices->t == tinyply::Type::FLOAT32){
            m_points.resize(vertices->count);
            std::memcpy(m_points.data(), vertices->buffer.get(), numVerticesBytes);
            if(m_recenter)
            {
                glm::dvec3 sum;
                for (const auto &i : m_points)
                {
                    sum += i;
                }
                sum /= m_points.size();
                for (auto &i : m_points)
                {
                    i -= sum;
                }
            }
        }else if(vertices->t == tinyply::Type::FLOAT64){
            std::vector<glm::dvec3> points;
            points.resize(vertices->count);
            m_points.resize(vertices->count);
            std::memcpy(points.data(), vertices->buffer.get(), numVerticesBytes);
            if(m_recenter)
            {
                glm::dvec3 sum = glm::dvec3(0);
                for (const auto &i : points)
                {
                    sum += i / (double)points.size();
                }
                for (auto &i : points)
                {
                    i -= sum;
                }
            }
            for(int i = 0; i < vertices->count; i++){
                m_points[i].x = points[i].x;
                m_points[i].y = points[i].z;
                m_points[i].z = points[i].y;
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
    ImGui::Checkbox("Recenter when load", &m_recenter);
    ImGui::Text(("Original amount: " + std::to_string(m_points.size())).c_str());
    ImGui::DragFloat("Point size", &m_pointSize, 0.01f, 0.01f, 100.0f);
    ImGui::DragFloat("Compress factor", &m_compressFactor, 0.001f, 0.0001f, 10.0f);
    ImGui::DragFloat("Compress scale", &m_scale, 0.001f, 0.0001f, 10.0f);

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
    FileUtils::SaveFile(("SaveInternal Compressed to PLY##Particles"), "PointCloud", {".ply"}, [&](const std::filesystem::path &filePath) {
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
    const auto owner = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), "Compressed Point Cloud");
    auto particles = owner.GetOrSetPrivateComponent<Particles>().lock();
    particles->m_material = AssetManager::CreateAsset<Material>();
    particles->m_material.Get<Material>()->SetProgram(DefaultResources::GLPrograms::StandardInstancedProgram);
    particles->m_mesh = DefaultResources::Primitives::Cube;
    auto particleMatrices = particles->m_matrices;
    auto &matrices = particleMatrices->m_value;
    auto compressed = std::vector<glm::vec3>();
    Compress(compressed);
    matrices.resize(compressed.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(compressed[i]) * glm::scale(glm::vec3(m_pointSize));
    }
    particleMatrices->Update();
}
void PointCloud::Compress(std::vector<glm::vec3>& points)
{
    RecalculateBoundingBox();
    if (m_compressFactor == 0)
    {
        UNIENGINE_ERROR("Resolution invalid!");
        return;
    }
    points.clear();

    float xMin = m_boundingBox.m_min.x - std::fmod(m_boundingBox.m_min.x, m_compressFactor) -
        (m_boundingBox.m_min.x < 0 ? m_compressFactor : 0);
    float yMin = m_boundingBox.m_min.y - std::fmod(m_boundingBox.m_min.y, m_compressFactor) -
        (m_boundingBox.m_min.y < 0 ? m_compressFactor : 0);
    float zMin = m_boundingBox.m_min.z - std::fmod(m_boundingBox.m_min.z, m_compressFactor) -
        (m_boundingBox.m_min.z < 0 ? m_compressFactor : 0);

    float xMax = m_boundingBox.m_max.x - std::fmod(m_boundingBox.m_max.x, m_compressFactor) +
        (m_boundingBox.m_max.x > 0 ? m_compressFactor : 0);
    float yMax = m_boundingBox.m_max.y - std::fmod(m_boundingBox.m_max.y, m_compressFactor) +
        (m_boundingBox.m_max.y > 0 ? m_compressFactor : 0);
    float zMax = m_boundingBox.m_max.z - std::fmod(m_boundingBox.m_max.z, m_compressFactor) +
        (m_boundingBox.m_max.z > 0 ? m_compressFactor : 0);

    UNIENGINE_LOG(
        "X, Y, Z MIN: [" + std::to_string(xMin) + ", " + std::to_string(yMin) + ", " + std::to_string(zMin) + "]");
    UNIENGINE_LOG(
        "X, Y, Z MAX: [" + std::to_string(xMax) + ", " + std::to_string(yMax) + ", " + std::to_string(zMax) + "]");

    std::vector<int> voxels;
    int rangeX = ((xMax - xMin) / m_compressFactor);
    int rangeY = ((yMax - yMin) / m_compressFactor);
    int rangeZ = ((zMax - zMin) / m_compressFactor);
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
        int posX = ((i.x - m_boundingBox.m_min.x) / m_compressFactor);
        int posY = ((i.y - m_boundingBox.m_min.y) / m_compressFactor);
        int posZ = ((i.z - m_boundingBox.m_min.z) / m_compressFactor);
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
                    points.push_back((glm::vec3(x, y, z) * m_compressFactor) * m_scale);
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
    out << YAML::Key << "m_recenter" << m_recenter;
    out << YAML::Key << "m_pointSize" << m_pointSize;
    out << YAML::Key << "m_compressFactor" << m_compressFactor;
    out << YAML::Key << "m_scale" << m_scale;
    out << YAML::Key << "m_boundingBox.m_min" << m_boundingBox.m_min;
    out << YAML::Key << "m_boundingBox.m_max" << m_boundingBox.m_max;
    if (!m_points.empty())
    {
        out << YAML::Key << "m_points" << YAML::Value
            << YAML::Binary((const unsigned char *)m_points.data(), m_points.size() * sizeof(glm::vec3));
    }
}
void PointCloud::Deserialize(const YAML::Node &in)
{
    if(in["m_recenter"]) m_recenter = in["m_recenter"].as<bool>();
    m_pointSize = in["m_pointSize"].as<float>();
    m_scale = in["m_scale"].as<float>();
    m_compressFactor = in["m_compressFactor"].as<float>();
    if(in["m_boundingBox.m_min"]) m_boundingBox.m_min = in["m_boundingBox.m_min"].as<glm::vec3>();
    if(in["m_boundingBox.m_max"]) m_boundingBox.m_max = in["m_boundingBox.m_max"].as<glm::vec3>();
    if (in["m_points"])
    {
        auto vertexData = in["m_points"].as<YAML::Binary>();
        m_points.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_points.data(), vertexData.data(), vertexData.size());
    }
}
void PointCloud::ApplyOriginal()
{
    const auto owner = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), "Original Point Cloud");
    auto particles = owner.GetOrSetPrivateComponent<Particles>().lock();
    particles->m_material = AssetManager::CreateAsset<Material>();
    particles->m_material.Get<Material>()->SetProgram(DefaultResources::GLPrograms::StandardInstancedProgram);
    particles->m_mesh = DefaultResources::Primitives::Cube;
    auto particleMatrices = particles->m_matrices;
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
        {"x", "z", "y"},
        Type::FLOAT32,
        m_points.size(),
        reinterpret_cast<uint8_t *>(m_points.data()),
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

