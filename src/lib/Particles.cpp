#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <Particles.hpp>
#include <RenderManager.hpp>

#include <Tinyply.hpp>
using namespace tinyply;

using namespace UniEngine;

void Particles::OnCreate()
{
    m_matrices = std::make_shared<PointCloud>();
    m_boundingBox = Bound();
    SetEnabled(true);
}

void Particles::RecalculateBoundingBox()
{
    if (m_matrices->m_value.empty())
    {
        m_boundingBox.m_min = glm::vec3(0.0f);
        m_boundingBox.m_max = glm::vec3(0.0f);
        return;
    }
    glm::vec3 minBound = glm::vec3(static_cast<int>(INT_MAX));
    glm::vec3 maxBound = glm::vec3(static_cast<int>(INT_MIN));
    auto meshBound = m_mesh.Get<Mesh>()->GetBound();
    for (auto &i : m_matrices->m_value)
    {
        glm::vec3 center = i * glm::vec4(meshBound.Center(), 1.0f);
        glm::vec3 size = glm::vec4(meshBound.Size(), 0) * i / 2.0f;
        minBound = glm::vec3(
            (glm::min)(minBound.x, center.x - size.x),
            (glm::min)(minBound.y, center.y - size.y),
            (glm::min)(minBound.z, center.z - size.z));

        maxBound = glm::vec3(
            (glm::max)(maxBound.x, center.x + size.x),
            (glm::max)(maxBound.y, center.y + size.y),
            (glm::max)(maxBound.z, center.z + size.z));
    }
    m_boundingBox.m_max = maxBound;
    m_boundingBox.m_min = minBound;
}

void Particles::OnGui()
{
    ImGui::Checkbox("Forward Rendering##Particles", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##Particles", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##Particles", &m_castShadow);
    ImGui::Text(("Instance count##Particles" + std::to_string(m_matrices->m_value.size())).c_str());
    if (ImGui::Button("Calculate bounds##Particles"))
    {
        RecalculateBoundingBox();
    }
    static bool displayBound;
    ImGui::Checkbox("Display bounds##Particles", &displayBound);
    if (displayBound)
    {
        static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
        ImGui::ColorEdit4("Color:##Particles", (float *)(void *)&displayBoundColor);
        const auto transform = GetOwner().GetDataComponent<GlobalTransform>().m_value;
        RenderManager::DrawGizmoMesh(
            DefaultResources::Primitives::Cube,
            displayBoundColor,
            transform * glm::translate(m_boundingBox.Center()) * glm::scale(m_boundingBox.Size()),
            1);
    }

    EditorManager::DragAndDropButton<Material>(m_material, "Material");
    EditorManager::DragAndDropButton<Mesh>(m_mesh, "Mesh");
    FileUtils::OpenFile(("Load PointCloud##Particles"), ".ply", [&](const std::filesystem::path &filePath) {
        try
        {
            m_matrices->Load(filePath);
            UNIENGINE_LOG("Loaded from " + filePath.string());
        }
        catch (std::exception &e)
        {
            UNIENGINE_ERROR("Failed to load from " + filePath.string());
        }
    });
}

void Particles::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
    out << YAML::Key << "m_castShadow" << m_castShadow;
    out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

    m_mesh.Save("m_mesh", out);
    m_material.Save("m_material", out);
    out << YAML::Key << "m_matrices" << YAML::BeginMap;
    m_matrices->Serialize(out);
    out << YAML::EndMap;
}

void Particles::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["m_forwardRendering"].as<bool>();
    m_castShadow = in["m_castShadow"].as<bool>();
    m_receiveShadow = in["m_receiveShadow"].as<bool>();

    m_mesh.Load("m_mesh", in);
    m_material.Load("m_material", in);

    m_matrices->Deserialize(in["m_matrices"]);
}
void Particles::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<Particles>(target);
}
void Particles::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_mesh);
    list.push_back(m_material);
}
void PointCloud::Serialize(YAML::Emitter &out)
{
    if (!m_value.empty())
    {
        out << YAML::Key << "m_value" << YAML::Value
            << YAML::Binary((const unsigned char *)m_value.data(), m_value.size() * sizeof(glm::mat4));
    }
}
void PointCloud::Deserialize(const YAML::Node &in)
{
    if (in["m_value"])
    {
        YAML::Binary vertexData = in["m_value"].as<YAML::Binary>();
        m_value.resize(vertexData.size() / sizeof(glm::mat4));
        std::memcpy(m_value.data(), vertexData.data(), vertexData.size());
    }
}
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
            std::vector<glm::vec3> verts(vertices->count);
            std::memcpy(verts.data(), vertices->buffer.get(), numVerticesBytes);
            m_value.resize(vertices->count);
            for (int i = 0; i < vertices->count; i++)
            {
                m_value[i] = glm::translate(verts[i]) * glm::scale(glm::vec3(1.0f));
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}
