#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>  
#include"JModelInterface.h"

namespace JinEngine
{ 
	struct JModelAttribute;
	struct ModelConstructionData;
	class JGameObject; 
	class JSkeletonAsset;
	class JScene; 

	class JModel : public JModelInterface
	{
		REGISTER_CLASS(JModel)
		struct ModelMetadata : public ObjectMetadata
		{
		public:
			struct GuidStruct
			{
			public:
				size_t meshGuid;
				size_t matGuid;
			};
			uint meshPartCount;
			std::vector<GuidStruct> partGuidVector;
		};
	private:
		JScene* modelScene;
		JGameObject* modelRoot;
		JGameObject* skeletonRoot; 
		std::vector<JGameObject*> meshPartCash;
		std::unique_ptr<JModelAttribute> modelAttribute = nullptr;
		JSkeletonAsset* skeletonAsset = nullptr;  
	public: 		   
		JSkeletonAsset* GetSkeletonAsset()noexcept;
		const uint GetTotalMeshCount()const noexcept;
		const uint GetTotalVertexCount()const noexcept;
		const uint GetTotalIndexCount()const noexcept;
		DirectX::XMFLOAT3 GetModelCenter()const noexcept;
		DirectX::XMFLOAT3 GetSkeletonCenter()const noexcept;
		float GetModelRadius()const noexcept;
		float GetSkeletonRadius()const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MODEL;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JModelSceneInterface* ModelSceneInterface() final;
	private:
		JScene* GetModelScene()noexcept final;
		JGameObject* GetModelRoot()noexcept final;
		JGameObject* GetSkeletonRoot()noexcept final;
	public:
		bool Copy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;    
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool IsValidResource()const noexcept;
	private:
		bool ReadObjModelData();
		bool ReadFbxModelData(); 
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		std::vector<JGameObject*>::const_iterator GetMeshPartVectorHandle(uint& count)noexcept;
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JModel* model);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JModel* model);
		static JModel* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, ModelMetadata& metadata);
		static void RegisterJFunc();
	private:
		JModel(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JModel();
	};
}