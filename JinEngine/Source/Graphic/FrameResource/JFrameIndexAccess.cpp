/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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