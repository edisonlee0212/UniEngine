#pragma once
#include <AssetManager.hpp>
#include <EditorManager.hpp>
#include <Entity.hpp>
#include <SerializationManager.hpp>
namespace UniEngine
{
class UNIENGINE_API ClassRegistry
{
  public:
    template <typename T = IAsset> static void RegisterAsset(const std::string &name)
    {
        AssetManager::RegisterAssetType<T>(name);
    }
    template <typename T = IDataComponent> static void RegisterDataComponent(const std::string &name)
    {
        SerializationManager::RegisterDataComponentType<T>(name);
    }
    template <typename T = IPrivateComponent> static void RegisterPrivateComponent(const std::string &name)
    {
        SerializationManager::RegisterSerializableType<T>(name);
        EditorManager::RegisterPrivateComponent<T>();
    }
    template <typename T = ISystem> static void RegisterSystem(const std::string &name)
    {
        SerializationManager::RegisterSerializableType<T>(name);
    }

};

template <typename T = IDataComponent> class UNIENGINE_API DataComponentRegistration
    {
      public:
        DataComponentRegistration(const std::string &name)
        {
            ClassRegistry::RegisterDataComponent<T>(name);
        }
    };

template <typename T = ISerializable> class UNIENGINE_API AssetRegistration
    {
      public:
        AssetRegistration(const std::string &name)
        {
            ClassRegistry::RegisterAsset<T>(name);
        }
    };

template <typename T = ISerializable> class UNIENGINE_API PrivateComponentRegistration
    {
      public:
        PrivateComponentRegistration(const std::string &name)
        {
            ClassRegistry::RegisterPrivateComponent<T>(name);
        }
    };

template <typename T = ISerializable> class UNIENGINE_API SystemRegistration
    {
      public:
        SystemRegistration(const std::string &name)
        {
            ClassRegistry::RegisterSystem<T>(name);
        }
    };

} // namespace UniEngine
