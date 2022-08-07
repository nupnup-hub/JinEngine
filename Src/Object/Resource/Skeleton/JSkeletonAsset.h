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
	public:
		JSkeleton* GetSkeleton()noexcept;
		JAvatar* GetAvatar()noexcept;
		JSKELETON_TYPE GetSkeletonType()noexcept;
		std::string GetJointName(int index)noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SKELETON;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;

		void SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept;
		bool HasAvatar()noexcept;
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept; 
	public:
		JSkeletonAssetAvatarInterface* AvatarInterface() final;
	private:
		void SetAvatar(JAvatar* avatar)noexcept final;
		void CopyAvatarJointIndex(JAvatar* target)noexcept final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool ReadSkeletonAssetData();
	private:
		void SetSkeleton(JSkeleton&& skeleon);
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JSkeletonAsset* skeletonAsset);
		static JSkeletonAsset* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterFunc();
	private:	
		JSkeletonAsset(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JSkeletonAsset();
	};
}