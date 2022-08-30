#pragma once
#include"JSkeletonAssetInterface.h"
#include"JSkeletonType.h"
#include"../../../Core/JDataType.h" 
#include<memory>

namespace JinEngine
{
	struct JSkeleton;
	class JAvatar;
	class JSkeletonAsset : public JSkeletonAssetInterface
	{
		REGISTER_CLASS(JSkeletonAsset)
	private:
		std::unique_ptr<JSkeleton> skeleton = nullptr;
		std::unique_ptr<JAvatar> avatar = nullptr;
		JSKELETON_TYPE skeletonType;

		size_t ownerModelGuid;
		bool isValidOwnerModelGuid = false;
	public:
		JSkeleton* GetSkeleton()noexcept;
		JAvatar* GetAvatar()noexcept;
		JSKELETON_TYPE GetSkeletonType()const noexcept;
		size_t GetOwnerModelGuid()const noexcept;
		std::wstring GetJointName(int index)noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SKELETON;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;

		void SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept;
		bool HasAvatar()noexcept;
		bool HasValidModelGuid()noexcept;
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept; 
	public:
		JSkeletonAssetAvatarInterface* AvatarInterface() final;
		JSkeletonAssetModelInteface* ModelInteface() final;
	private:
		void SetAvatar(JAvatar* avatar)noexcept final;
		void CopyAvatarJointIndex(JAvatar* target)noexcept final;
	public:
		bool Copy(JObject* ori);
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool ReadSkeletonAssetData();
	private:
		void SetSkeleton(JSkeleton&& skeleon);
		void SetOwnerModelGuid(const size_t modelGuid)noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JSkeletonAsset* skeletonAsset);
		static JSkeletonAsset* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterJFunc();
	private:	
		JSkeletonAsset(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JSkeletonAsset();
	};
}