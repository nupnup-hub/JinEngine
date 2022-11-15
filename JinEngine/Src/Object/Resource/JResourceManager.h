#pragma once 
#include<vector>
#include<functional> 
#include<algorithm>
#include<type_traits>
#include"JResourceType.h" 
#include"JResourceUserInterface.h" 
#include"JResourceManagerInterface.h"
#include"JResourceEventType.h" 
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderFunctionEnum.h"
#include"Texture/JDefaulTextureType.h"  

#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/Event/JEventManager.h"
#include"../../Core/Storage/JStorage.h"
#include"../../Core/File/JFileIOResult.h"

namespace JinEngine
{   
	class JMeshGeometry;
	class JMaterial;
	class JTexture;
	class JShader; 
	class JResourceIO;
	class JResourceData;

	namespace Core
	{
		template <typename T> class JCreateUsingNew;
	}

	class JResourceManagerImpl : public JDirectoryStorageInterface, 
		public Core::JEventManager<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
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
			bool AddResource(JResourceObject* resource)noexcept;
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
			bool Add(JDirectory* dir)noexcept;
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

		std::unique_ptr<JResourceData> resourceData;
		std::unique_ptr<JResourceIO> resourceIO; 
	public: 
		JMeshGeometry* GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)noexcept;
		JMaterial* GetDefaultMaterial(const J_DEFAULT_MATERIAL type)noexcept;
		JTexture* GetDefaultTexture(const J_DEFAULT_TEXTURE type)noexcept;
		JShader* GetDefaultShader(const J_DEFAULT_GRAPHIC_SHADER type)noexcept;
		JShader* GetDefaultShader(const J_DEFAULT_COMPUTE_SHADER type)noexcept;
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
		JResourceMangerAppInterface* AppInterface() final;
		JResourceStorageInterface* ResourceStorageInterface() final;
		JDirectoryStorageInterface* DirectoryStorageInterface() final;
		JEventInterface* EvInterface() final;
	private:
		void Initialize()final;
		void Terminate()final;
		void StoreProjectResource()final;
		void LoadSelectorResource()final;
		void LoadProjectResource()final;
		void DeleteUnuseMetafile();
	private:
		bool AddResource(JResourceObject& newResource)noexcept final;
		bool RemoveResource(JResourceObject& resource)noexcept final; 
		bool AddJDirectory(JDirectory& newDirectory)noexcept final;
		bool RemoveJDirectory(JDirectory& dir)noexcept final;
	private:
		void DestroyUnusedResource(const J_RESOURCE_TYPE rType, bool isIgnreUndestroyableFlag);
	private: 
		//Create Default Resource  
		void CreateDefaultTexture(const std::vector<J_DEFAULT_TEXTURE>& textureType);
		void CreateDefaultShader();
		void CreateDefaultMaterial();
		void CreateDefaultMesh();
		//bool IsOverlappedFilepath(const std::string& name, const std::string& path) noexcept;
	private:
		JResourceManagerImpl();
		~JResourceManagerImpl();
	public:
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		uint GetResourceCount()
		{
			return rCash.find(T::GetStaticResourceType())->second.Count();
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		T* GetResource(const size_t guid)noexcept
		{ 
			return static_cast<T*>(rCash.find(T::GetStaticResourceType())->second.Get(guid));
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		T* GetResourceByPath(const std::wstring& path)noexcept
		{
			return static_cast<T*>(rCash.find(T::GetStaticResourceType())->second.GetByPath(path));
		}
		template<typename T, typename ...Param, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		T* GetResourceByCondition(Core::JFunctor<bool, T*, Param...> functor, Param... var)
		{
			std::vector<JResourceObject*>& rvec = rCash.find(T::GetStaticResourceType())->second.GetVector();
			const uint rVecCount = (uint)rvec.size();
			for (uint i = 0; i < rVecCount; ++i)
			{ 
				T* rObj = static_cast<T*>(rvec[i]);
				if (functor(rObj, std::forward<Param>(var)...))
					return rObj;
			}
			return nullptr;
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
