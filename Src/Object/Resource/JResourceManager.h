#pragma once 
#include<vector>
#include<functional> 
#include<algorithm>
#include"JResourceType.h"
#include"JResourceData.h"   
#include"Scene/Preview/PreviewSceneGroup.h" 
#include"../Directory/JDirectoryStorage.h"  

namespace JinEngine
{  
	struct ResourceHandleStruct; 
	class JMeshGeometry;
	class JMaterial;
	class JTexture;
	class JShader;
	class JDirectory;
	class JResourceIO;
	class PreviewSceneGroup;
	class PreviewScene; 
	 
	class JResourceManager 
	{
	private:
		class Storage
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
			JResourceObject* GetByPath(const std::string& path)noexcept;
			std::vector<JResourceObject*>::const_iterator GetVectorIter(uint& count);
			bool Has(const size_t guid)noexcept;
			uint Count()const noexcept;
			JResourceObject* AddResource(JResourceObject* resource)noexcept;
			bool EraseResource(JResourceObject* resource)noexcept;
			void Clear();
		};
	private:
		using ResourceCash = std::unordered_map<J_RESOURCE_TYPE, Storage>;
		using DirectoryCash = JDirectoryStorage;
	private:
		static std::unique_ptr<JResourceManager> instance;
		static size_t managerGuid;

		ResourceCash rCashMap;
		JDirectoryStorage dCash;

		std::vector<std::unique_ptr<PreviewSceneGroup>> previewSceneGroup;
		std::vector<PreviewSceneGroup*> previewSceneGroupCashVec;
		JScene* nowMainScene; 
		JDirectory* engineRootDir;
		JDirectory* projectRootDir; 

		JResourceData resourceData;
		std::unique_ptr<JResourceIO> resourceIO; 
	public:
		JResourceManager();
		~JResourceManager();
		static JResourceManager& Instance(); 
		void LoadSelectorResource();
		void LoadProjectResource();
		void Terminate();
		 
		JMeshGeometry* GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)noexcept;
		JMaterial* GetDefaultMaterial(const J_DEFAULT_MATERIAL materialType)noexcept;
		JTexture* GetEditorTexture(const J_EDITOR_TEXTURE enumName)noexcept;
		JShader* GetDefaultShader(const J_DEFAULT_SHADER shaderType)noexcept;  
		JDirectory* GetDirectory(const std::string& path)noexcept;
		JDirectory* GetEditorResourceDirectory()noexcept;
		JDirectory* GetActivatedDirectory()noexcept;  
		JScene* GetMainScene()noexcept;

		uint GetResourceCount(const J_RESOURCE_TYPE type)noexcept;
		JResourceObject* GetResource(const J_RESOURCE_TYPE type, const size_t guid)noexcept;
		JResourceObject* GetResourceByPath(const J_RESOURCE_TYPE type, const std::string& path)noexcept;
		std::vector<JResourceObject*>::const_iterator GetResourceVectorHandle(const J_RESOURCE_TYPE type, uint& resouceCount)noexcept;

		bool HasResource(const J_RESOURCE_TYPE rType, const size_t guid)noexcept;
		bool EraseResource(JResourceObject* resource)noexcept;
		//Create Default Resource  
 
		std::vector<PreviewSceneGroup*>::const_iterator GetPreviewGroupVectorHandle(_Out_ uint& groupCount)noexcept;
		PreviewSceneGroup* CreatePreviewGroup(const std::string& ownerName, const uint groupSceneCapacity = 0)noexcept;
		bool ClearPreviewGroup(PreviewSceneGroup* group)noexcept;
		bool ErasePreviewGroup(PreviewSceneGroup* group)noexcept; 
		PreviewScene* CreatePreviewScene(PreviewSceneGroup* group, JResourceObject* resourceObject, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag = PREVIEW_FLAG::NONE)noexcept;
		bool ErasePreviewScene(PreviewSceneGroup* group, JResourceObject* resource)noexcept;
	private: 
		void CreateDefaultTexture(const std::vector<JResourceData::DefaultTextureInfo>& textureInfo)noexcept;
		void CreateDefaultShader()noexcept;
		void CreateDefaultMaterial()noexcept;
		void CreateDefaultMesh();
		  
		JResourceObject* GetResourceByType(const J_RESOURCE_TYPE type, const size_t guid)noexcept;
		void StoreResourceData();
		void LoadObject();
		//bool IsOverlappedFilepath(const std::string& name, const std::string& path) noexcept;
	public:

	public:
		template<typename T>
		uint GetResourceCount()
		{
			return rCashMap.find(T::GetStaticResourceType())->second.Count();
		}
		template<typename T>
		T* GetResource(const size_t guid)noexcept
		{ 
			return static_cast<T*>(rCashMap.find(T::GetStaticResourceType())->second.Get(guid));
		}
		template<typename T>
		T* GetResourceByPath(const std::string& path)noexcept
		{
			return static_cast<T*>(rCashMap.find(T::GetStaticResourceType())->second.GetByPath(path));
		}
		template<typename T, typename R = typename std::vector<JResourceObject*>::const_iterator>
		R GetResourceVectorHandle(uint& resouceCount)noexcept
		{
			return rCashMap.find(T::GetStaticResourceType())->second.GetVectorIter(resouceCount);
		}
	private:
		template<typename T, typename C = std::enable_if_t<std::is_base_of<JObject, T>::value>>
		static bool ObjectNameCompare(T* a, T* b)noexcept
		{
			return a->GetName() < b->GetName();
		}
	};
}

/*
template<> bool DoEraseResource<JMaterial>(JMaterial* resource)noexcept
		{
			uint index;
			if (material->EraseResource(resource, index))
			{
				uint count;
				std::vector<JMaterial*>::const_iterator iter = GetResourceVectorHandle<JMaterial>(count);
				for (uint i = index; i < count; ++i)
					(*(iter + i))->SetMatCBIndex(i);

				return true;
			}
			else
				return false;
		}
		template<> bool DoEraseResource<JTexture>(JTexture* resource)noexcept
		{
			if (texture->HasResource(resource->GetGuid()))
			{
				uint count;
				std::vector<JMaterial*>::const_iterator iter = GetResourceVectorHandle<JMaterial>(count);
				for (uint i = 0; i < count; ++i)
					(*(iter + i))->PopTexture(resource);

				uint index;
				return texture->EraseResource(resource, index);
			}
			else
				return false;

		}
		template<> bool DoEraseResource<JModel>(JModel* resource)noexcept
		{
			EraseResource<JScene>(resource->GetModelScene());
			uint index;
			return model->EraseResource(resource, index);
		}
		template<> bool DoEraseResource<JDirectory>(JDirectory* resource)noexcept
		{
			resource->Clear();
			uint index = 0;
			return directory->EraseResource(resource, index);
		}
*/