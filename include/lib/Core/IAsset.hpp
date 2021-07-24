#pragma once
#include <uniengine_export.h>
#include <ISerializable.hpp>
namespace UniEngine
{
class Texture2D;
class UNIENGINE_API IAsset : public ISerializable
{
  protected:
    friend class EditorManager;
    friend class AssetManager;
    std::shared_ptr<Texture2D> m_icon;
    size_t m_typeId = 0;
  public:
    virtual void OnCreate();
    std::string m_name;
    [[nodiscard]] virtual size_t GetHashCode() const;
};

inline void IAsset::OnCreate()
{
    m_name = "New Resource";
}

inline size_t IAsset::GetHashCode() const
{
    return reinterpret_cast<size_t>(this);
}
} // namespace UniEngine
