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


#include"JFrustumCulling.h"
#include"../JCullingInfo.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../../Object/Component/JComponent.h"
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JScenePrivate.h"

namespace JinEngine
{
	namespace Graphic
	{
		JFrustumCulling::~JFrustumCulling()
		{
			Clear();
		}
		void JFrustumCulling::Initialize()
		{}
		void JFrustumCulling::Clear()
		{}
		void JFrustumCulling::FrustumCulling(const JUserPtr<JScene>& scene, const JDrawHelper& helper)
		{ 
			if (helper.cam != nullptr)
			{ 
				JAcceleratorCullingInfo info(helper.cam->CullingUserInterface(), helper.cam->GetBoundingFrustum());
				info.allowCullingOrderedByDistance = true;
				info.allowPushVisibleObjVec = true;
				info.appAlignedObjVec = &helper.objVec.common; 
				JScenePrivate::CullingInterface::ViewCulling(scene, info);
				 
				const uint camFrustumIndex = helper.GetCullInterface().GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
				auto& alignedVec = helper.objVec.aligned[camFrustumIndex];
				if (alignedVec.size() > 0)
					alignedVec.clear();

				alignedVec.insert(alignedVec.begin(), 
					std::make_move_iterator(info.appAlignedObjVec->begin()),
					std::make_move_iterator(info.appAlignedObjVec->begin() + info.pushedCount));
			}
			else
				JScenePrivate::CullingInterface::ViewCulling(scene, helper.lit);
		}
	}
}