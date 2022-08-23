#pragma once 
#include"../../JEditorWindow.h"

namespace JinEngine
{
	class JSkeletonAsset;
	class JGameObject;

	namespace Editor
	{
		class JAvatarDetail : public JEditorWindow
		{
		private:
			JSkeletonAsset* targetAsset;
			JGameObject* targetObj;
		public:
			JAvatarDetail(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JAvatarDetail() = default;
			JAvatarDetail(const JAvatarDetail& rhs) = delete;
			JAvatarDetail& operator=(const JAvatarDetail& rhs) = delete;

			void Initialize(JEditorUtility* editorUtility)noexcept;
			void ActivateAvatarEdit(JSkeletonAsset* targetAsset, JGameObject* targetObj)noexcept;
			void UpdateWindow(JEditorUtility* editorUtility)override;
		};
	}
}