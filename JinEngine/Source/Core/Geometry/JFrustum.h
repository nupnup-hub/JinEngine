#pragma once
#include"JGeometryEnum.h"
#include"../Math/JVector.h"
#include"../Math/JMatrix.h"

//개발 보류
//추후에 DirectX math, DirectX collision wrapping class 만들때 다시 개발.
namespace JinEngine
{
	namespace Core
	{
		class JFrustum
		{
		public:
			static const size_t conerCount = 8;
		public:
			JVector3F origin;            // Origin of the frustum (and projection).
			JVector4F orientation;       // Quaternion representing rotation.
		public:
			float rightSlope;           // Positive X (X/Z)
			float leftSlope;            // Negative X
			float topSlope;             // Positive Y (Y/Z)
			float bottomSlope;          // Negative Y
			float nearRange;					// Z of the near plane and far plane.
			float farRange;
		public:
			JFrustum();
			JFrustum(const JVector3F& origin, const JVector4F& orientation,
				const float rightSlope, const float leftSlope,
				const float topSlope, const float bottonSlope,
				const float nearRange, const float farRange);
			JFrustum(const JMatrix4x4& m);
		public:
			JFrustum Transform(const JMatrix4x4& m) const;
			JFrustum Transform(const float scale, const JVector4F& rotation, const JVector3F& translation) const;
		public:
			void GetCorners(_Out_writes_(conerCount) JVector3F* corners) const;
		};
	}
}

//정확도 문제가 있으므로 추후에 필요할시 수정해서 사용
/*
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
			JCullingFrustum(_In_ const JFrustum& frustum);
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
*/