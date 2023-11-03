#include"JAcceleratorNode.h"  
#include"../../../Component/RenderItem/JRenderItemPrivate.h"  
#include"../../../../Core/Geometry/JDirectXCollisionEx.h"

using namespace DirectX;
namespace JinEngine
{
	ContainmentType JAcceleratorNode::Contain(const JAcceleratorCullingInfo& info, const BoundingBox& box)const noexcept
	{
		return info.useBBox ? info.bbox.Contains(box) : info.frustum.Contains(box);
	}
	ContainmentType JAcceleratorNode::Contain(const JAcceleratorCullingInfo& info, const BoundingOrientedBox& box)const noexcept
	{
		return info.useBBox ? info.bbox.Contains(box) : info.frustum.Contains(box);
	}
	/*
	Core::J_CULLING_RESULT JAcceleratorNode::Contain(const JAcceleratorCullingInfo& info, const BoundingBox& box, _Inout_ Core::J_CULLING_FLAG& flag)const noexcept
	{
		return info.useBBox ? Core::JCullingFrustum::ConvertType(info.bbox.Contains(box)) : info.jFrustum.Contain(box, flag);
	}
	Core::J_CULLING_RESULT JAcceleratorNode::Contain(const JAcceleratorCullingInfo& info, const BoundingOrientedBox& box, _Inout_ Core::J_CULLING_FLAG& flag)const noexcept
	{
		return info.useBBox ? Core::JCullingFrustum::ConvertType(info.bbox.Contains(box)) : info.jFrustum.Contain(box, flag);
	}
	*/
	bool JAcceleratorNode::IsIntersectCullingFrustum(const JAcceleratorCullingInfo& info, const BoundingOrientedBox& box)const noexcept
	{
		return info.hasCullingArea && info.cullingFrustum.Contains(box) != ContainmentType::DISJOINT;
	}
	bool JAcceleratorNode::IsIntersectCullingFrustum(const JAcceleratorCullingInfo& info, const BoundingBox& box)const noexcept
	{
		return info.hasCullingArea && info.cullingFrustum.Contains(box) != ContainmentType::DISJOINT;
	}
	void JAcceleratorNode::SetCulling(JAcceleratorCullingInfo& info, const JUserPtr<JRenderItem>& rItem)noexcept
	{
		using RItemFrameIndexInteface = JRenderItemPrivate::FrameIndexInterface;
		info.cullUser.SetCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
	}
	void JAcceleratorNode::OffCulling(JAcceleratorCullingInfo& info, const JUserPtr<JRenderItem>& rItem)noexcept
	{
		using RItemFrameIndexInteface = JRenderItemPrivate::FrameIndexInterface;
		info.cullUser.OffCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
	}
	bool JAcceleratorNode::IsContain(const JAcceleratorContainInfo& info, const BoundingBox& bbox)const noexcept
	{
		ContainmentType res = ContainmentType::DISJOINT;
		if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::FRUSTUM)
			res = info.frustum.Contains(bbox);
		else if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::OBB)
			res = info.obb.Contains(bbox);
		return info.allowIntersect ? res != ContainmentType::DISJOINT : res == ContainmentType::CONTAINS;
	}
	bool JAcceleratorNode::IsContain(const JAcceleratorContainInfo& info, const BoundingOrientedBox& bbox)const noexcept
	{
		ContainmentType res = ContainmentType::DISJOINT;
		if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::FRUSTUM)
			res = info.frustum.Contains(bbox);
		else if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::OBB)
			res = info.obb.Contains(bbox);
		return info.allowIntersect ? res != ContainmentType::DISJOINT : res == ContainmentType::CONTAINS;
	}
}