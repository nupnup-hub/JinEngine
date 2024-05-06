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
			static DirectX::BoundingBox CreateBoundingBox(const JVector3<float>& vMin, const JVector3<float>& vMax)noexcept;
			static DirectX::BoundingBox CreateBoundingBox(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept;
			static DirectX::BoundingSphere CreateBoundingSphere(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept;
			//not use 대신 obb사용
			//static DirectX::BoundingBox CreateBoundingBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents, const DirectX::XMVECTOR q)noexcept;
		};
	}
}