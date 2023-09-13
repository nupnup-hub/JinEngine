#pragma once
#include"JAcceleratorOption.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JRenderItem;
	namespace Graphic
	{
		class JCullingUserInterface;
	}
	class JAcceleratorNode
	{
	protected:
		DirectX::ContainmentType Contain(const JAcceleratorCullingInfo& info, const DirectX::BoundingBox& box)const noexcept;
		DirectX::ContainmentType Contain(const JAcceleratorCullingInfo& info, const DirectX::BoundingOrientedBox& box)const noexcept;
		Core::J_CULLING_RESULT Contain(const JAcceleratorCullingInfo& info, const DirectX::BoundingBox& box, _Inout_ Core::J_CULLING_FLAG& flag)const noexcept;
		Core::J_CULLING_RESULT Contain(const JAcceleratorCullingInfo& info, const DirectX::BoundingOrientedBox& box, _Inout_ Core::J_CULLING_FLAG& flag)const noexcept;
	protected:
		bool IsIntersectCullingFrustum(const JAcceleratorCullingInfo& info, const DirectX::BoundingOrientedBox& box)const noexcept;
		bool IsIntersectCullingFrustum(const JAcceleratorCullingInfo& info, const DirectX::BoundingBox& box)const noexcept;
	protected:
		void SetCulling(JAcceleratorCullingInfo& info, const JUserPtr<JRenderItem>& rItem)noexcept;
		void OffCulling(JAcceleratorCullingInfo& info, const JUserPtr<JRenderItem>& rItem)noexcept;
	protected:
		bool IsContain(const JAcceleratorContainInfo& info, const DirectX::BoundingBox& bbox)const noexcept;
		bool IsContain(const JAcceleratorContainInfo& info, const DirectX::BoundingOrientedBox& bbox)const noexcept;
	};
}