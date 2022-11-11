#pragma once
#include"../JResourceObject.h"  
#include"../JClearableInterface.h"
#include<memory>

namespace JinEngine
{
	class JAvatar; 
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
	 
	class JSkeletonAssetInterface : public JResourceObject, 
		public JSkeletonAssetAvatarInterface,
		public JClearableInterface
	{
	protected:
		JSkeletonAssetInterface(const JResourceObject::JResourceInitData& initdata);
	};
}