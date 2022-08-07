#pragma once
#include"../JResourceObject.h"  
#include"../JClearableInterface.h"

namespace JinEngine
{
	class JAvatar;
	class AvatarEditor;
	class JSkeletonAssetAvatarInterface : public JResourceObject
	{
	private:
		friend class AvatarEditor;
	protected:
		JSkeletonAssetAvatarInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSkeletonAssetAvatarInterface* AvatarInterface() = 0;
	private:
		virtual void SetAvatar(JAvatar* avatar)noexcept = 0;
		virtual void CopyAvatarJointIndex(JAvatar* target)noexcept = 0;
	};
	class JSkeletonAssetInterface : public JSkeletonAssetAvatarInterface, public JClearableInterface
	{
	protected:
		JSkeletonAssetInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}