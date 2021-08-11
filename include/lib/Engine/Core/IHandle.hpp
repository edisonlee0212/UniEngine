#pragma once
#include <uniengine_export.h>
#include <xhash>
namespace UniEngine
{
struct UNIENGINE_API Handle
{
    friend class IAsset;
    Handle();
    Handle(uint64_t value);
    Handle(const Handle &other);

    operator uint64_t()
    {
        return m_value;
    }
    operator const uint64_t() const
    {
        return m_value;
    }
    [[nodiscard]] uint64_t GetValue() const {
        return m_value;
    }
  private:
    uint64_t m_value;
};
class UNIENGINE_API IHandle{
    friend class EntityManager;
    friend class EditorManager;
    friend class AssetManager;
    friend class DefaultResources;
    friend class SerializationManager;
    friend class IAsset;
    Handle m_handle;
  public:
    Handle GetHandle()
    {
        return m_handle;
    }
};
} // namespace UniEngine

namespace std
{
template <>
struct hash<UniEngine::Handle>
    {
    size_t operator()(const UniEngine::Handle &handle) const
    {
        return hash<uint64_t>()((uint64_t)handle);
    }
    };
}