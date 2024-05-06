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


#include"JDirectXCollisionEx.h"  

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		DirectX::BoundingBox JDirectXCollisionEx::Union(_In_ DirectX::BoundingBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept
		{
			XMFLOAT3 minF;
			XMFLOAT3 maxF;
			BoundingBoxMinMax(box, minF, maxF);

			minF.x = XMMin(minF.x, point.x);
			minF.y = XMMin(minF.y, point.y);
			minF.z = XMMin(minF.z, point.z);

			maxF.x = XMMax(maxF.x, point.x);
			maxF.y = XMMax(maxF.y, point.y);
			maxF.z = XMMax(maxF.z, point.z);

			XMVECTOR min = XMLoadFloat3(&minF);
			XMVECTOR max = XMLoadFloat3(&maxF);

			XMFLOAT3 center;
			XMFLOAT3 extent;
			XMStoreFloat3(&center, 0.5f * (min + max));
			XMStoreFloat3(&extent, 0.5f * (max - min));

			return DirectX::BoundingBox(center, extent);
		}
		DirectX::BoundingBox JDirectXCollisionEx::Union(_In_ const DirectX::BoundingBox& b1, _In_ const DirectX::BoundingBox& b2)noexcept
		{
			if (b1.Extents.x == 0 && b1.Extents.y == 0 && b1.Extents.z == 0)
				return b2;
			else if (b2.Extents.x == 0 && b2.Extents.y == 0 && b2.Extents.z == 0)
				return b1;

			XMFLOAT3 b1MinF;
			XMFLOAT3 b1MaxF;
			BoundingBoxMinMax(b1, b1MinF, b1MaxF);

			XMFLOAT3 b2MinF;
			XMFLOAT3 b2MaxF;
			BoundingBoxMinMax(b2, b2MinF, b2MaxF);

			XMFLOAT3 minF;
			XMFLOAT3 maxF;

			minF.x = XMMin(b1MinF.x, b2MinF.x);
			minF.y = XMMin(b1MinF.y, b2MinF.y);
			minF.z = XMMin(b1MinF.z, b2MinF.z);

			maxF.x = XMMax(b1MaxF.x, b2MaxF.x);
			maxF.y = XMMax(b1MaxF.y, b2MaxF.y);
			maxF.z = XMMax(b1MaxF.z, b2MaxF.z);

			XMVECTOR min = XMLoadFloat3(&minF);
			XMVECTOR max = XMLoadFloat3(&maxF);

			XMFLOAT3 center;
			XMFLOAT3 extent;
			XMStoreFloat3(&center, 0.5f * (min + max));
			XMStoreFloat3(&extent, 0.5f * (max - min));

			return DirectX::BoundingBox(center, extent);
		}
		void JDirectXCollisionEx::BoundingBoxMinMax(_In_ const DirectX::BoundingBox& box,
			_Out_ DirectX::XMFLOAT3& min,
			_Out_ DirectX::XMFLOAT3& max)noexcept
		{ 
			DirectX::XMFLOAT3 coners[8];
			box.GetCorners(coners);

			XMVECTOR minV = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
			XMVECTOR maxV = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

			for (uint i = 0; i < 8; ++i)
			{
				minV = XMVectorMin(minV, XMLoadFloat3(&coners[i]));
				maxV = XMVectorMax(maxV, XMLoadFloat3(&coners[i]));
			}
			XMStoreFloat3(&min, minV);
			XMStoreFloat3(&max, maxV);
		}
		float JDirectXCollisionEx::SurfaceArea(_In_ DirectX::BoundingBox& box)noexcept
		{
			XMFLOAT3 v(XMFLOAT3(box.Extents.x * 2, box.Extents.y * 2, box.Extents.z * 2));
			return 2.f * (v.x * v.y + v.x * v.z + v.y * v.z);
		}
		DirectX::BoundingBox JDirectXCollisionEx::Convert(_In_ const DirectX::BoundingFrustum& frustum)noexcept
		{
			XMFLOAT3 coners[8];
			frustum.GetCorners(coners);

			XMFLOAT3 minF(FLT_MAX, FLT_MAX, FLT_MAX);
			XMFLOAT3 maxF(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (uint i = 0; i < 8; ++i)
			{
				minF.x = XMMin(minF.x, coners[i].x);
				minF.y = XMMin(minF.y, coners[i].y);
				minF.z = XMMin(minF.z, coners[i].z);

				maxF.x = XMMax(maxF.x, coners[i].x);
				maxF.y = XMMax(maxF.y, coners[i].y);
				maxF.z = XMMax(maxF.z, coners[i].z);
			}
			XMVECTOR minV = XMLoadFloat3(&minF);
			XMVECTOR maxV = XMLoadFloat3(&maxF);
			DirectX::BoundingBox boundBox;
			XMStoreFloat3(&boundBox.Center, 0.5f * (maxV + minV));
			XMStoreFloat3(&boundBox.Extents, 0.5f * (maxV - minV));
			return boundBox;
		}
		DirectX::BoundingBox JDirectXCollisionEx::CreateBoundingBox(const JVector3<float>& vMin, const JVector3<float>& vMax)noexcept
		{
			DirectX::BoundingBox box;
			box.Center = ((vMin + vMax) * 0.5f).ToSimilar<XMFLOAT3>();
			box.Extents = ((vMax - vMin) * 0.5f).ToSimilar<XMFLOAT3>();
			return box;
		}
		DirectX::BoundingBox JDirectXCollisionEx::CreateBoundingBox(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept
		{
			DirectX::BoundingBox box;
			XMStoreFloat3(&box.Center, 0.5f * (vMin + vMax));
			XMStoreFloat3(&box.Extents, 0.5f * (vMax - vMin));
			return box;
		}
		DirectX::BoundingSphere JDirectXCollisionEx::CreateBoundingSphere(const DirectX::XMVECTOR vMin, const DirectX::XMVECTOR vMax)noexcept
		{
			DirectX::BoundingSphere sphere;
			XMStoreFloat3(&sphere.Center, 0.5f * (vMin + vMax)); 
			sphere.Radius = XMVectorGetX(XMVector3Length(vMax - (0.5f * (vMin + vMax))));
			return sphere;
		}
	}
}
//not use 대신 obb사용
/*
		DirectX::BoundingBox JDirectXCollisionEx::CreateBoundingBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents, const DirectX::XMVECTOR q)noexcept
		{
			XMFLOAT3 coners[8]
			{
				XMFLOAT3(extents.x, extents.y, extents.z),
				XMFLOAT3(extents.x, extents.y, -extents.z),
				XMFLOAT3(extents.x, -extents.y, extents.z),
				XMFLOAT3(extents.x, -extents.y, -extents.z),
				XMFLOAT3(-extents.x, extents.y, extents.z),
				XMFLOAT3(-extents.x, extents.y, -extents.z),
				XMFLOAT3(-extents.x, -extents.y, extents.z),
				XMFLOAT3(-extents.x, -extents.y, -extents.z)
			};

			XMVECTOR minV = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
			XMVECTOR maxV = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

			for (uint i = 0; i < 8; ++i)
			{
				XMVECTOR conerV = XMVector3Rotate(XMLoadFloat3(&coners[i]), q);
				minV = XMVectorMin(minV, conerV);
				maxV = XMVectorMax(maxV, conerV);
			}
			DirectX::BoundingBox boundBox;
			XMStoreFloat3(&boundBox.Center, 0.5f * (maxV + minV));
			XMStoreFloat3(&boundBox.Extents, 0.5f * (maxV - minV));
			return boundBox;
		}
*/