#include"JSpaceSpatialNode.h"
#include"../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../Graphic/Culling/JCullingInterface.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using RItemFrameIndexInteface = JRenderItemPrivate::FrameIndexInterface;
		}
		void JSpaceSpatialNode::SetCulling(Graphic::JCullingUserInterface& cullUser, const JUserPtr<JRenderItem>& rItem)const noexcept
		{
			cullUser.SetCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
		}
		void JSpaceSpatialNode::OffCulling(Graphic::JCullingUserInterface& cullUser, const JUserPtr<JRenderItem>& rItem)const noexcept
		{
			cullUser.OffCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
		}
	}
}