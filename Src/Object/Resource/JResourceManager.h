#pragma once 
#include<vector>
#include<functional> 
#include<algorithm>
#include"JResourceType.h"
#include"JResourceData.h"   
#include"JResourceUserInterface.h" 
#include"JResourceManagerInterface.h"
#include"JResourceEventType.h" 
#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/Event/JEventManager.h"
#include"../../Core/Storage/JStorage.h"
#include"../../Core/File/JFileIOResult.h"

namespace JinEngine
{  
	struct ResourceHandleStruct; 
	class JMeshGeometry;
	class JMaterial;
	class JTexture;
	class JShader; 
	class JResourceIO;
	 
	namespace Core
	{
		template <typename T> class JCreateUsingNew;
	}

	class JResourceManagerImpl : public JDirectoryStorageInterface, public Core::JEventManager<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
	{
		REGISTER_CLASS(JResourceManagerImpl)
	private:
		template <typename T> friend class Core::JCreateUsingNew;
	private:
		class ResourceStorage
		{
		private:
			using ResourceVector = Core::JVectorStorage<JResourceObject>;
			using ResourceMap = Core::JMapStorage<JResourceObject, size_t>;
			using StoreInfoVec = std::vector<Core::JFileIOResultInfo>;
		private:
			ResourceVector rVec;
			ResourceMap rMap;
		public:
			JResourceObject* Get(const size_t guid)noexcept;
			JResourceObject* GetByIndex(const uint index);
			JResourceObject* GetByPath(const std::wstring& path)noexcept;
			std::vector<JResourceObject*>& GetVector();
			std::vector<JResourceObject*>::const_iterator GetVectorIter(uint& count);
			bool Has(const size_t guid)noexcept;
			uint Count()const noexcept;
			JResourceObject* AddResource(JResourceObject* resource)noexcept;
			bool RemoveResource(JResourceObject& resource)noexcept;
			void Clear();
		};
		class DirectoryStorage
		{
		private:
			using ResourceVector = Core::JVectorStorage<JDirectory>;
			using ResourceMap = Core::JMapStorage<JDirectory, size_t>;
			using StoreInfoVec = std::vector<Core::JFileIOResultInfo>;
		private:
			ResourceVector dVec;
			ResourceMap dMap;
		public:
			uint Count()const noexcept;
			JDirectory* Get(const uint index);
			JDirectory* GetByGuid(const size_t guid);
			JDirectory* GetByPath(const std::wstring& path);
			JDirectory* GetOpenDirectory();
			JDirectory* Add(JDirectory* dir)noexcept;
			bool Remove(JDirectory* dir)noexcept;
			void Clear();
		};
	private:
		using ResourceCash = std::unordered_map<J_RESOURCE_TYPE, ResourceStorage>;
		using DirectoryCash = DirectoryStorage;
	private: 
		size_t managerGuid;

		ResourceCash rCash;
		DirectoryCash dCash;
		 
		JDirectory* engineRootDir;
		JDirectory* projectRootDir; 

		JResourceData resourceData;
		std::unique_ptr<JResourceIO> resourceIO; 
	public: 
		JMeshGeometry* GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)noexcept;
		JMaterial* GetDefaultMaterial(const J_DEFAULT_MATERIAL materialType)noexcept;
		JTexture* GetEditorTexture(const J_EDITOR_TEXTURE enumName)noexcept;
		JShader* GetDefaultShader(const J_DEFAULT_SHADER shaderType)noexcept;  
		JDirectory* GetDirectory(const size_t guid)noexcept;
		JDirectory* GetDirectory(const std::wstring& path)noexcept;
		JDirectory* GetEditorResourceDirectory()noexcept;
		JDirectory* GetActivatedDirectory()noexcept;   

		uint GetResourceCount(const J_RESOURCE_TYPE type)noexcept;
		JResourceObject* GetResource(const J_RESOURCE_TYPE type, const size_t guid)noexcept;
		JResourceObject* GetResourceByPath(const J_RESOURCE_TYPE type, const std::wstring& path)noexcept;
		std::vector<JResourceObject*>::const_iterator GetResourceVectorHandle(const J_RESOURCE_TYPE type, uint& resouceCount)noexcept;

		bool HasResource(const J_RESOURCE_TYPE rType, const size_t guid)noexcept;
	public:
		JResourceMangerOwnerInterface* OwnerInterface() final;
		JResourceStorageInterface* ResourceStorageInterface() final;
		JDirectoryStorageInterface* DirectoryStorageInterface() final;
		JEventInterface* EvInterface() final;
	private:
		void LoadSelectorResource()final;
		void LoadProjectResource()final;
		void Terminate()final;
		void DeleteUnuseMetafile();
	private:
		JResourceObject* AddResource(JResourceObject& newResource);
		JDirectory* AddDirectory(JDirectory& newDirectory);
	private:
		bool RemoveResource(JResourceObject& resource)noexcept final; 
		bool RemoveJDirectory(JDirectory& dir)noexcept final;
	private:
		bool DestroyUnusedResource(const J_RESOURCE_TYPE rType, bool isIgnreUndestroyableFlag);

	private: 
		//Create Default Resource  
		void CreateDefaultTexture(const std::vector<JResourceData::DefaultTextureInfo>& textureInfo)noexcept;
		void CreateDefaultShader()noexcept;
		void CreateDefaultMaterial()noexcept;
		void CreateDefaultMesh();

		void StoreResourceData();
		void LoadObject();
		//bool IsOverlappedFilepath(const std::string& name, const std::string& path) noexcept;
	private:
		static void RegisterJFunc();
		void RegistEvCallable();
	private:
		JResourceManagerImpl();
		~JResourceManagerImpl();
	public:
		template<typename T>
		uint GetResourceCount()
		{
			return rCash.find(T::GetStaticResourceType())->second.Count();
		}
		template<typename T>
		T* GetResource(const size_t guid)noexcept
		{ 
			return static_cast<T*>(rCash.find(T::GetStaticResourceType())->second.Get(guid));
		}
		template<typename T>
		T* GetResourceByPath(const std::wstring& path)noexcept
		{
			return static_cast<T*>(rCash.find(T::GetStaticResourceType())->second.GetByPath(path));
		}
		template<typename T>
		auto GetResourceVectorHandle(uint& resouceCount) noexcept -> typename std::vector<JResourceObject*>::const_iterator
		{
			return rCash.find(T::GetStaticResourceType())->second.GetVectorIter(resouceCount);
		}
	private:
		template<typename T, typename C = std::enable_if_t<std::is_base_of<JObject, T>::value>>
		static bool ObjectNameCompare(T* a, T* b)noexcept
		{
			return a->GetName() < b->GetName();
		}
	};
	using JResourceManager = Core::JSingletonHolder<JResourceManagerImpl>;
}
