#pragma once
#include <ISerializable.hpp>
#include <uniengine_export.h>
#include <xhash>
namespace UniEngine
{
struct UNIENGINE_API AssetHandle
{
    AssetHandle();
    AssetHandle(uint64_t value);
    AssetHandle(const AssetHandle& other);

    operator uint64_t () { return m_value; }
    operator const uint64_t () const { return m_value; }
  private:
    uint64_t m_value;
};

class Texture2D;
class UNIENGINE_API IAsset : public ISerializable
{
    friend class EditorManager;
    friend class AssetManager;
    friend class DefaultResources;
    AssetHandle m_handle;
  protected:
    std::shared_ptr<Texture2D> m_icon;
  public:
    AssetHandle GetHandle()
    {
        return m_handle;
    }
    virtual void OnCreate();
    std::string m_name;
};

inline void IAsset::OnCreate()
{
    m_name = "New Asset";
}

} // namespace UniEngine
namespace std
{
template <>
struct hash<UniEngine::AssetHandle>
{
    size_t operator()(const UniEngine::AssetHandle &handle) const
    {
        return hash<uint64_t>()((uint64_t)handle);
    }
};
}