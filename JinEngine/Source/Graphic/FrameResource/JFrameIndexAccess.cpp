#include"JFrameIndexAccess.h"
#include"../../Object/Component/JComponent.h" 
#include"../../Object/Component/Camera/JCameraPrivate.h" 
#include"../../Object/Component/Light/JLightPrivate.h"  
#include"../../Object/Component/Light/JLight.h"  
#include"../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../Object/Component/Animator/JAnimatorPrivate.h"
#include"../../Object/Resource/Scene/JScenePrivate.h"

namespace JinEngine::Graphic
{
	namespace
	{
		using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;  
		using CamFrameIndexInterface = JCameraPrivate::FrameIndexInterface;
		using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
		using AniFrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;
		using LitFrameIndexInterface = JLightPrivate::FrameIndexInterface;
	}

	int JFrameIndexAccess::GetSceneFrameIndex(JScene* scene)const noexcept
	{
		return SceneFrameIndexInterface::GetFrameIndex(scene); 
	}
	int JFrameIndexAccess::GetCamFrameIndex(JCamera* cam, const uint frameLayerIndex)const noexcept
	{
		return CamFrameIndexInterface::GetFrameIndex(cam, frameLayerIndex);
	}  
	int JFrameIndexAccess::GetLitFrameIndex(JLight* lit, const uint frameLayerIndex)const noexcept
	{ 
		return static_cast<JLightPrivate*>(&lit->PrivateInterface())->GetFrameIndexInterface().GetFrameIndex(lit, frameLayerIndex);
	} 
	int JFrameIndexAccess::GetLitShadowFrameIndex(JLight* lit)const noexcept
	{
		JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
		auto& frameUpdateInterface = lp->GetFrameIndexInterface();
		return frameUpdateInterface.GetFrameIndex(lit, frameUpdateInterface.GetShadowFrameLayerIndex(lit));
	}
	int JFrameIndexAccess::GetObjectFrameIndex(JRenderItem* rItem)const noexcept
	{
		return RItemFrameIndexInterface::GetObjectFrameIndex(rItem);
	}
	int JFrameIndexAccess::GetBoundingFrameIndex(JRenderItem* rItem)const noexcept
	{
		return RItemFrameIndexInterface::GetBoundingFrameIndex(rItem);
	}
	int JFrameIndexAccess::GetRefInfoFrameIndex(JRenderItem* rItem)const noexcept
	{
		return RItemFrameIndexInterface::GetRefInfoFrameIndex(rItem);
	}
	int JFrameIndexAccess::GetAnimationFrameIndex(JAnimator* ani)const noexcept
	{
		return AniFrameIndexInterface::GetFrameIndex(ani);
	}
}