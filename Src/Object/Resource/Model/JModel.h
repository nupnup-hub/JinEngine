#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"../JResourceObject.h"  

namespace JinEngine
{ 
	struct JModelAttribute;
	struct ModelConstructionData;
	class JGameObject; 
	class JSkeletonAsset;
	class JScene; 

	class JModel : public JResourceObject
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
		const JModelAttribute* GetModelData()const noexcept; 
		const uint GetTotalMeshCount()const noexcept;
		const uint GetTotalVertexCount()const noexcept;
		const uint GetTotalIndexCount()const noexcept;
		DirectX::XMFLOAT3 GetModelCenter()const noexcept;
		DirectX::XMFLOAT3 GetSkeletonCenter()const noexcept;
		float GetModelRadius()const noexcept;
		float GetSkeletonRadius()const noexcept;
		JScene* GetModelScene()const noexcept;
		JGameObject* GetModelRoot()const noexcept;
		JGameObject* GetSkeletonRoot()const noexcept;
		JSkeletonAsset* GetSkeletonAsset()const noexcept;  
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MODEL;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;    
	private:
		std::vector<JGameObject*>::const_iterator GetMeshPartVectorHandle(uint& count)noexcept;
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JModel* model);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JModel* model);
		static JModel* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::string& folderPath, ModelMetadata& metadata);
		static void RegisterFunc();
	private:
		JModel(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JModel();
	};
}