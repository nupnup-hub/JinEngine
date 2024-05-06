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
		info.cullUser.SetCulling(Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::RENDERITEM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
	}
	void JAcceleratorNode::OffCulling(JAcceleratorCullingInfo& info, const JUserPtr<JRenderItem>& rItem)noexcept
	{
		using RItemFrameIndexInteface = JRenderItemPrivate::FrameIndexInterface;
		info.cullUser.OffCulling(Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::RENDERITEM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
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