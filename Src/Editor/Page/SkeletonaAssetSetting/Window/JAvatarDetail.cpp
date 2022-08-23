#include"JAvatarDetail.h"
#include"../../JEditorAttribute.h"

namespace JinEngine
{
	namespace Editor
	{
		JAvatarDetail::JAvatarDetail(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid)
		{

		}
		void JAvatarDetail::Initialize(JEditorUtility* editorUtility)noexcept
		{

		}
		void JAvatarDetail::ActivateAvatarEdit(JSkeletonAsset* targetAsset, JGameObject* targetObj)noexcept
		{
			JAvatarDetail::targetAsset = targetAsset;
			JAvatarDetail::targetObj = targetObj;
		}
		void JAvatarDetail::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);
		}
	}
}