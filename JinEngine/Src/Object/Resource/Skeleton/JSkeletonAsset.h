#pragma once
#include"JSkeletonAssetInterface.h"
#include"JSkeletonType.h"
#include"../../../Core/JDataType.h" 
#include<memory>

namespace JinEngine
{
	struct JSkeleton;
	class JAvatar;
	class JSkeletonAsset final : public JSkeletonAssetInterface
	{
		REGISTER_CLASS(JSkeletonAsset)
	public:
		struct JSkeletonInitData : public JResourceInitData
		{
		public:
			std::unique_ptr<JSkeleton> skeleton;
		public:
			JSkeletonInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JSkeletonAsset>(GetAvailableFormat()[0]),
				std::unique_ptr<JSkeleton> skeleton = nullptr);
			JSkeletonInitData(const std::wstring& name,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JSkeletonAsset>(GetAvailableFormat()[0]),
				std::unique_ptr<JSkeleton> skeleton = nullptr);
			JSkeletonInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const std::wstring& oriPath,
				std::unique_ptr<JSkeleton> skeleton = nullptr);
			JSkeletonInitData(const std::wstring& name,
				JDirectory* directory,
				const std::wstring& oriPath,
				std::unique_ptr<JSkeleton> skeleton = nullptr);
		public:
			//For create new object
			//except load
			bool IsValidCreateData()final; 
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
		using InitData = JSkeletonInitData;
	private:
		struct JSkeletonAssetMetaData : public JResourceMetaData
		{
		public:
			std::unique_ptr<JAvatar> avatar = nullptr;
			JSKELETON_TYPE skeletonType ;
			size_t skeletonHash = 0;
		};
	private:
		std::unique_ptr<JSkeleton> skeleton = nullptr;
		std::unique_ptr<JAvatar> avatar = nullptr;
		JSKELETON_TYPE skeletonType;
		size_t skeletonHash = 0;
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SKELETON;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JSkeleton* GetSkeleton()noexcept;
		JAvatar* GetAvatar()noexcept;
		JSKELETON_TYPE GetSkeletonType()const noexcept; 
		size_t GetSkeletonHash()const noexcept;
		std::wstring GetJointName(int index)noexcept;
		std::vector<std::vector<uint8>> GetSkeletonTreeIndexVec()noexcept;

		void SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept;
		bool HasAvatar()noexcept; 
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept; 
	public:
		JSkeletonAssetAvatarInterface* AvatarInterface() final; 
	private:
		void SetAvatar(JAvatar* avatar)noexcept final;
		void CopyAvatarJointIndex(JAvatar* target)noexcept final;
		void SetSkeleton(std::unique_ptr<JSkeleton> skeleton); 
	private:
		void DoCopy(JObject* ori);
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool WriteSkeletonAssetData();
		bool ReadSkeletonAssetData(); 
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JSkeletonAsset* skeletonAsset);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JSkeletonAsset* skeletonAsset);
		static JSkeletonAsset* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);	   
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JSkeletonAssetMetaData& metadata);
		static void RegisterJFunc();
	private:	
		JSkeletonAsset(JSkeletonInitData& initdata); 
		~JSkeletonAsset();
	};
}