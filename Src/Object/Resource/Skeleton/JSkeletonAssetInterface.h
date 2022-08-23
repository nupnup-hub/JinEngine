#pragma once
#include"../JResourceObject.h"  
#include"../JClearableInterface.h"

namespace JinEngine
{
	class JAvatar;
	class JModel;
	namespace Editor
	{
		class JAvatarEditor;
	}

	class JSkeletonAssetAvatarInterface
	{
	private:
		friend class Editor::JAvatarEditor;
	protected:
		virtual ~JSkeletonAssetAvatarInterface() = default;
	public:
		virtual JSkeletonAssetAvatarInterface* AvatarInterface() = 0;
	private:
		virtual void SetAvatar(JAvatar* avatar)noexcept = 0;
		virtual void CopyAvatarJointIndex(JAvatar* target)noexcept = 0;
	};

	class JSkeletonAssetModelInteface
	{
	private:
		friend class JModel;
	protected:
		virtual ~JSkeletonAssetModelInteface() = default;
	public:
		virtual JSkeletonAssetModelInteface* ModelInteface() = 0;
	private:
		virtual void SetOwnerModelGuid(const size_t guid)noexcept = 0;
	};

	class JSkeletonAssetInterface : public JResourceObject, 
		public JSkeletonAssetAvatarInterface,
		public JSkeletonAssetModelInteface,
		public JClearableInterface
	{
	protected:
		JSkeletonAssetInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}