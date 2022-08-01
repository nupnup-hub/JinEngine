#pragma once 
#include"../EditorWindow.h"

namespace JinEngine
{
	class JSkeletonAsset;
	class JGameObject;

	class AvatarDetail : public EditorWindow
	{
	private: 
		JSkeletonAsset* targetAsset;
		JGameObject* targetObj;
	public:
		AvatarDetail(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~AvatarDetail() = default; 
		AvatarDetail(const AvatarDetail& rhs) = delete;
		AvatarDetail& operator=(const AvatarDetail& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept;
		void ActivateAvatarEdit(JSkeletonAsset* targetAsset, JGameObject* targetObj)noexcept;
		void UpdateWindow(EditorUtility* editorUtility)override;
	};
}