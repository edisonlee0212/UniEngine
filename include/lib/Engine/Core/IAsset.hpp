#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class Texture2D;
class UNIENGINE_API IAsset : public ISerializable
{
  protected:
    friend class EditorManager;
    friend class AssetManager;
    friend class DefaultResources;
    std::shared_ptr<Texture2D> m_icon;
    std::filesystem::path m_path;
  public:
    virtual void OnCreate();
    std::string m_name;

    void Save();
    void Load();
    bool m_saved;
};

inline void IAsset::OnCreate()
{
    m_name = "New Asset";
}
} // namespace UniEngine
