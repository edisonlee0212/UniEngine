#include <IAsset.hpp>
#include <DefaultResources.hpp>
#include <AssetManager.hpp>
using namespace UniEngine;
void IAsset::Save()
{
    if(m_handle.m_value < DefaultResources::GetMaxHandle().m_value) return;
    if(m_path.empty()) return;
    Save(m_path);
    m_saved = true;
}
void IAsset::Load()
{
    if(m_handle.m_value < DefaultResources::GetMaxHandle().m_value) return;
    if(m_path.empty()) return;
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
