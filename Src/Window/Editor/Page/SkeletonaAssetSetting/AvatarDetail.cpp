#include"AvatarDetail.h"
#include"../EditorAttribute.h"

namespace JinEngine
{
	AvatarDetail::AvatarDetail(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{

	}
	void AvatarDetail::Initialize(EditorUtility* editorUtility)noexcept
	{

	}
	void AvatarDetail::ActivateAvatarEdit(JSkeletonAsset* targetAsset, JGameObject* targetObj)noexcept
	{
		AvatarDetail::targetAsset = targetAsset;
		AvatarDetail::targetObj = targetObj;
	}
	void AvatarDetail::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
	}
}