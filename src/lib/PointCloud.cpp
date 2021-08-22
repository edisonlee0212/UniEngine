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
            m_finalOutput = m_points;
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
    ImGui::Text(("Point amount: " + std::to_string(m_points.size())).c_str());
    ImGui::Text(("Final amount: " + std::to_string(m_finalOutput.size())).c_str());
    ImGui::DragFloat("Point size", &m_pointSize, 0.01f, 0.01f, 100.0f);
    static float compressScale = 50.0f;
    ImGui::DragFloat("Compress scale", &compressScale, 0.001f, 0.0001f, 10.0f);
    if (ImGui::Button("Compress"))
    {
        Compress(compressScale);
    }
    if (ImGui::Button("Apply to particles"))
    {
        ApplyToParticles();
    }
    FileUtils::OpenFile(("Load PointCloud##Particles"), ".ply", [&](const std::filesystem::path &filePath) {
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
}
void PointCloud::ApplyToParticles()
{
    const auto owner = GetOwner();
    auto &matrices = owner.GetOrSetPrivateComponent<Particles>().lock()->m_matrices->m_value;
    matrices.resize(m_finalOutput.size());
    for (int i = 0; i < matrices.size(); i++)
    {
        matrices[i] = glm::translate(m_finalOutput[i]) * glm::scale(glm::vec3(m_pointSize));
    }
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
                 (m_boundingBox.m_min.x > 0 ? resolution : 0);
    float yMax = m_boundingBox.m_max.y - std::fmod(m_boundingBox.m_max.y, resolution) +
                 (m_boundingBox.m_min.x > 0 ? resolution : 0);
    float zMax = m_boundingBox.m_max.z - std::fmod(m_boundingBox.m_max.z, resolution) +
                 (m_boundingBox.m_min.x > 0 ? resolution : 0);

    UNIENGINE_LOG("X, Y, Z MIN: [" + std::to_string(xMin) + ", " + std::to_string(yMin) + ", " + std::to_string(zMin) + "]");
    UNIENGINE_LOG("X, Y, Z MAX: [" + std::to_string(xMax) + ", " + std::to_string(yMax) + ", " + std::to_string(zMax) + "]");
    std::vector<int> voxels;
    int rangeX = ((xMax - xMin) / resolution);
    int rangeY = ((yMax - yMin) / resolution);
    int rangeZ = ((zMax - zMin) / resolution);
    int voxelSize = rangeX * rangeY * rangeZ;

    if(voxelSize > m_points.size()){
        UNIENGINE_ERROR("Resolution too small: " + std::to_string(voxelSize));
        return;
    }else{
        UNIENGINE_LOG("Voxel size: " + std::to_string(voxelSize));
    }

    voxels.resize(voxelSize);
    memset(voxels.data(), 0, voxelSize * sizeof(int));

    for (const auto& i : m_points)
    {
        int posX = (i.x - m_boundingBox.m_min.x) / resolution;
        int posY = (i.y - m_boundingBox.m_min.y) / resolution;
        int posZ = (i.z - m_boundingBox.m_min.z) / resolution;
        auto index = posX * rangeY * rangeZ + posY * rangeZ + posZ;
        if(index >= voxelSize) continue;
        voxels[index]++;
    }
    m_finalOutput.clear();
    for(int i = 0; i < voxels.size(); i++){
        if(voxels[i] != 0){
            int posX = i / (rangeY * rangeZ);
            int posY = i / (rangeZ) % rangeX;
            int posZ = i % (rangeX * rangeY);
            m_finalOutput.push_back(glm::vec3(posX, posY, posZ) * resolution + m_boundingBox.m_min);
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
    if (!m_finalOutput.empty())
    {
        out << YAML::Key << "m_finalOutput" << YAML::Value
        << YAML::Binary((const unsigned char *)m_finalOutput.data(), m_finalOutput.size() * sizeof(glm::vec3));
    }
}
void PointCloud::Deserialize(const YAML::Node &in)
{
    m_pointSize = in["m_pointSize"].as<float>();
    if (in["m_points"])
    {
        YAML::Binary vertexData = in["m_points"].as<YAML::Binary>();
        m_points.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_points.data(), vertexData.data(), vertexData.size());
    }
    if (in["m_finalOutput"])
    {
        YAML::Binary vertexData = in["m_finalOutput"].as<YAML::Binary>();
        m_finalOutput.resize(vertexData.size() / sizeof(glm::vec3));
        std::memcpy(m_finalOutput.data(), vertexData.data(), vertexData.size());
    }
}
