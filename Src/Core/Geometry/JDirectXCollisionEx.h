#pragma once
#include<DirectXCollision.h>
#include"JBBox.h"

namespace JinEngine
{
	namespace Core
	{
		class JDirectXCollisionEx
		{
		public:
			static DirectX::BoundingBox Union(_In_ DirectX::BoundingBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept;
			static DirectX::BoundingBox Union(_In_ const DirectX::BoundingBox& b1, _In_ const DirectX::BoundingBox& b2)noexcept;
			static void BoundingBoxMinMax(_In_ const DirectX::BoundingBox& box,
				_Out_ DirectX::XMFLOAT3& min,
				_Out_ DirectX::XMFLOAT3& max)noexcept;
			static float SurfaceArea(_In_ DirectX::BoundingBox& box)noexcept;
			static DirectX::BoundingBox Convert(_In_ const DirectX::BoundingFrustum& frustum)noexcept;
			static DirectX::BoundingBox CreateBoundingBox(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept;
			static DirectX::BoundingSphere CreateBoundingSphere(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept;
		};
	}
}