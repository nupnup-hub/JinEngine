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