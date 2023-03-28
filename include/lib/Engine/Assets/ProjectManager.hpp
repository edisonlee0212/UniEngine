#pragma once
#include <IAsset.hpp>
#include <ISingleton.hpp>
#include "Serialization.hpp"
namespace UniEngine
{
	class Folder;
	class UNIENGINE_API AssetRecord
	{
		friend class Folder;
		friend class IAsset;
		std::string m_assetFileName;
		std::string m_assetExtension;
		std::string m_assetTypeName;
		Handle m_assetHandle = 0;
		std::weak_ptr<IAsset> m_asset;
		std::weak_ptr<Folder> m_folder;
		std::weak_ptr<AssetRecord> m_self;

	public:
		[[nodiscard]] std::weak_ptr<Folder> GetFolder() const;
		[[nodiscard]] Handle GetAssetHandle() const;
		[[nodiscard]] std::shared_ptr<IAsset> GetAsset();
		[[nodiscard]] std::string GetAssetTypeName() const;
		[[nodiscard]] std::string GetAssetFileName() const;
		[[nodiscard]] std::string GetAssetExtension() const;
		void DeleteMetadata() const;

		[[nodiscard]] std::filesystem::path GetProjectRelativePath() const;
		[[nodiscard]] std::filesystem::path GetAbsolutePath() const;
		void SetAssetFileName(const std::string& newName);
		void SetAssetExtension(const std::string& newExtension);

		void Save() const;
		void Load(const std::filesystem::path& path);
	};

	class UNIENGINE_API Folder
	{
		friend class IAsset;
		friend class EditorLayer;
		friend class ProjectManager;
		std::string m_name;
		std::unordered_map<Handle, std::shared_ptr<AssetRecord>> m_assetRecords;
		std::map<Handle, std::shared_ptr<Folder>> m_children;
		std::weak_ptr<Folder> m_parent;
		Handle m_handle = 0;
		std::weak_ptr<Folder> m_self;
		void Refresh(const std::filesystem::path& parentAbsolutePath);
		void RegisterAsset(const std::shared_ptr<IAsset>& asset, const std::string& fileName, const std::string& extension);
	public:
		bool IsSelfOrAncestor(const Handle& handle);
		void DeleteMetadata() const;
		[[nodiscard]] Handle GetHandle() const;
		[[nodiscard]] std::filesystem::path GetProjectRelativePath() const;
		[[nodiscard]] std::filesystem::path GetAbsolutePath() const;
		[[nodiscard]] std::string GetName() const;

		void Rename(const std::string& newName);

		void MoveChild(const Handle& childHandle, const std::shared_ptr<Folder>& dest);
		void DeleteChild(const Handle& childHandle);
		[[nodiscard]] std::weak_ptr<Folder> GetChild(const Handle& childHandle);
		[[nodiscard]] std::weak_ptr<Folder> GetOrCreateChild(const std::string& folderName);

		void MoveAsset(const Handle& assetHandle, const std::shared_ptr<Folder>& dest);
		void DeleteAsset(const Handle& assetHandle);
		[[nodiscard]] bool HasAsset(const std::string& fileName, const std::string& extension);
		[[nodiscard]] std::shared_ptr<IAsset> GetOrCreateAsset(const std::string& fileName, const std::string& extension);
		[[nodiscard]] std::shared_ptr<IAsset> GetAsset(const Handle& assetHandle);

		void Save() const;
		void Load(const std::filesystem::path& path);
		virtual ~Folder();
	};

	class Texture2D;
	struct UNIENGINE_API DefaultResource
	{
		std::string m_name;
		std::shared_ptr<IAsset> m_value;
	};

	class UNIENGINE_API AssetThumbnail {
		std::shared_ptr<Texture2D> m_icon;
	};

	class UNIENGINE_API ProjectManager : public ISingleton<ProjectManager>
	{
		friend class Application;
		friend class Editor;
		friend class EditorLayer;
		friend class AssetRecord;
		friend class Folder;
		friend class PhysicsLayer;
		std::shared_ptr<Folder> m_projectFolder;
		std::filesystem::path m_projectPath;
		std::optional<std::function<void()>> m_newSceneCustomizer;
		std::weak_ptr<Folder> m_currentFocusedFolder;
		std::unordered_map<Handle, std::shared_ptr<IAsset>> m_residentAsset;
		std::unordered_map<Handle, std::weak_ptr<IAsset>> m_assetRegistry;
		std::unordered_map<Handle, std::weak_ptr<AssetRecord>> m_assetRecordRegistry;
		std::unordered_map<Handle, std::weak_ptr<Folder>> m_folderRegistry;

		friend class ClassRegistry;
		std::unordered_map<std::string, std::unordered_map<Handle, DefaultResource>> m_defaultResources;
		std::shared_ptr<Scene> m_startScene;
		std::unordered_map<std::string, std::vector<std::string>> m_assetExtensions;
		std::map<std::string, std::string> m_typeNames;

		std::unordered_map<Handle, std::weak_ptr<AssetThumbnail>> m_assetThumbnails;
		std::vector<std::shared_ptr<AssetThumbnail>> m_assetThumbnailStorage;
		int m_maxThumbnailSize = 256;
		friend class DefaultResources;
		friend class AssetRegistry;
		friend class ProjectManager;
		friend class Editor;
		friend class EditorLayer;
		friend class IAsset;
		friend class Scene;
		friend class Prefab;
		template <typename T>
		static void RegisterAssetType(const std::string& name, const std::vector<std::string>& extensions);
		template <typename T>
		static std::shared_ptr<T> CreateDefaultResource(const Handle& handle, const std::string& name);
		std::shared_ptr<IAsset> static CreateDefaultResource(
			const std::string& typeName, const Handle& handle, const std::string& name);

		static void DisplayDefaultResources();
		[[nodiscard]] static std::shared_ptr<IAsset> CreateTemporaryAsset(const std::string& typeName);
		[[nodiscard]] static std::shared_ptr<IAsset> CreateTemporaryAsset(const std::string& typeName, const Handle& handle);

		static void FolderHierarchyHelper(const std::shared_ptr<Folder>& folder);

	public:
		static std::shared_ptr<IAsset> DuplicateAsset(const std::shared_ptr<IAsset>& target);
		std::shared_ptr<IAsset> m_inspectingAsset;
		bool m_showProjectWindow = true;
		bool m_showAssetInspectorWindow = true;
		bool m_showDefaultResourcesWindow = false;
		static std::weak_ptr<Scene> GetStartScene();
		static void SetStartScene(const std::shared_ptr<Scene>& scene);
		static void OnInspect();
		static void SaveProject();
		static void SetScenePostLoadActions(const std::function<void()>& actions);
		[[nodiscard]] static std::filesystem::path GenerateNewPath(const std::string& prefix, const std::string& postfix);
		[[nodiscard]] static std::weak_ptr<Folder> GetCurrentFocusedFolder();
		[[nodiscard]] static std::filesystem::path GetProjectPath();
		[[nodiscard]] static std::string GetProjectName();
		[[nodiscard]] static std::weak_ptr<Folder> GetOrCreateFolder(const std::filesystem::path& projectRelativePath);
		[[nodiscard]] static std::shared_ptr<IAsset> GetOrCreateAsset(const std::filesystem::path& projectRelativePath);
		[[nodiscard]] static std::shared_ptr<IAsset> GetAsset(const Handle& handle);
		[[nodiscard]] static std::weak_ptr<Folder> GetFolder(const Handle& handle);
		static void GetOrCreateProject(const std::filesystem::path& path);
		[[nodiscard]] static bool IsInProjectFolder(const std::filesystem::path& absolutePath);
		[[nodiscard]] static bool IsValidAssetFileName(const std::filesystem::path& path);
		template <typename T> [[nodiscard]] static std::shared_ptr<T> CreateTemporaryAsset();
		template <typename T> [[nodiscard]] static std::vector<std::string> GetExtension();
		[[nodiscard]] static std::vector<std::string> GetExtension(const std::string& typeName);
		[[nodiscard]] static std::string GetTypeName(const std::string& extension);
		[[nodiscard]] static bool IsAsset(const std::string& typeName);
		static void ScanProject();
		static void OnDestroy();
		[[nodiscard]] static std::filesystem::path GetPathRelativeToProject(const std::filesystem::path& absolutePath);
	};
	template <typename T> std::shared_ptr<T> ProjectManager::CreateTemporaryAsset()
	{
		return std::dynamic_pointer_cast<T>(CreateTemporaryAsset(Serialization::GetSerializableTypeName<T>()));
	}
	template <typename T>
	std::shared_ptr<T> ProjectManager::CreateDefaultResource(const Handle& handle, const std::string& name)
	{
		return std::dynamic_pointer_cast<T>(
			CreateDefaultResource(Serialization::GetSerializableTypeName<T>(), handle, name));
	}

	template <typename T>
	void ProjectManager::RegisterAssetType(const std::string& name, const std::vector<std::string>& extensions)
	{
		auto& projectManager = GetInstance();
		Serialization::RegisterSerializableType<T>(name);
		projectManager.m_defaultResources[name] = std::unordered_map<Handle, DefaultResource>();
		projectManager.m_assetExtensions[name] = extensions;
		for (const auto& extension : extensions)
		{
			projectManager.m_typeNames[extension] = name;
		}
	}
	template <typename T> std::vector<std::string> ProjectManager::GetExtension()
	{
		return GetExtension(Serialization::GetSerializableTypeName<T>());
	}
} // namespace UniEngine