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


#pragma once
#include"JGeometryEnum.h"
#include"../Math/JVector.h"
#include"../Math/JMatrix.h"

//���� ����
//���Ŀ� DirectX math, DirectX collision wrapping class ���鶧 �ٽ� ����.
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

//��Ȯ�� ������ �����Ƿ� ���Ŀ� �ʿ��ҽ� �����ؼ� ���
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