#include <AssetManager.hpp>
#include <DefaultResources.hpp>
#include <IAsset.hpp>
using namespace UniEngine;
void IAsset::Save()
{
    if (m_path.empty())
        return;
    Save(m_path);
    m_saved = true;
}
void IAsset::Load()
{
    if (m_path.empty())
        return;
    Load(m_path);
}
void IAsset::Save(const std::filesystem::path &path)
{
    auto directory = path;
    directory.remove_filename();
    std::filesystem::create_directories(directory);
    YAML::Emitter out;
    Serialize(out);
    std::ofstream fout(path.string());
    fout << out.c_str();
    fout.flush();
}
void IAsset::Load(const std::filesystem::path &path)
{
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    Deserialize(in);
}
IAsset::~IAsset()
{
}

bool AssetRef::Update()
{
    if (m_assetHandle.GetValue() == 0)
    {
        m_value.reset();
        return false;
    }
    else if(!m_value)
    {
        auto ptr = AssetManager::Get(m_assetTypeName, m_assetHandle);
        if(ptr){
            m_value = ptr;
            return true;
        }
        Clear();
        return false;
    }
    return true;
}

void AssetRef::Clear()
{
    m_value.reset();
    m_assetHandle = Handle(0);
}

void IAsset::OnCreate()
{
    m_name = "New " + m_typeName;
}