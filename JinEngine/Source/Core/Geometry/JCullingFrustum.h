#pragma once
#include<DirectXCollision.h>

namespace JinEngine
{
	/*
	For Culling
	*/
	namespace Core
	{
		enum class J_CULLING_RESULT
		{
			DISJOINT,
			INTERSECT,
			CONTAIN
		};
		enum J_CULLING_FLAG
		{
			NONE = 0,
			CULLING_FLAG_CONTAIN_FORWARD = 1 << 0,
			CULLING_FLAG_CONTAIN_UP = 1 << 1,
			CULLING_FLAG_CONTAIN_RIGHT = 1 << 2
		};
		class JCullingFrustum
		{
		private:
			DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 forward = DirectX::XMFLOAT3(0, 0, 1);
			DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1, 0, 0);
			DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0, 1, 0);

			float fNear = 0;
			float fFar = 0;
			float fovX = 0;
			float fovY = 0;
		public:
			JCullingFrustum() = default;
			JCullingFrustum(_In_ const DirectX::BoundingFrustum& frustum);
			~JCullingFrustum();
			JCullingFrustum(const JCullingFrustum& rhs) = default;
			JCullingFrustum& operator=(const JCullingFrustum& rhs) = default;
			JCullingFrustum(JCullingFrustum&& rhs) = default;
			JCullingFrustum& operator=(JCullingFrustum&& rhs) = default;
		public:
			J_CULLING_RESULT Contain(_In_ const DirectX::BoundingBox& box, _Inout_ J_CULLING_FLAG& flag)const noexcept;
			J_CULLING_RESULT Contain(_In_ const DirectX::BoundingOrientedBox& box, _Inout_ J_CULLING_FLAG& flag)const noexcept;
		public:
			static J_CULLING_RESULT ConvertType(const DirectX::ContainmentType t);
		};
	}
}