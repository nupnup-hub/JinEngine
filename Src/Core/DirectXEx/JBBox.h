#pragma once
#include"../JDataType.h"
#include<DirectXMath.h>
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Core
	{
		class JBBox
		{
		public:
			DirectX::XMFLOAT3 min;
			DirectX::XMFLOAT3 max;
		public:
			JBBox(const DirectX::XMFLOAT3& min = DirectX::XMFLOAT3(0, 0, 0), const DirectX::XMFLOAT3& max = DirectX::XMFLOAT3(0, 0, 0));
			JBBox(const DirectX::BoundingBox& boundBox);
			~JBBox();
			JBBox(const JBBox& rhs) = default;
			JBBox& operator=(const JBBox& rhs) = default;
			JBBox(JBBox&& rhs) = default;
			JBBox& operator=(JBBox&& rhs) = default;

			DirectX::BoundingBox Convert()const noexcept;
			DirectX::XMVECTOR Extent()const noexcept;
			DirectX::XMVECTOR Center()const noexcept;
			float Surface()const noexcept;
			/*
				Dimension
				x = 0, y = 1, z = 2
			*/
			uint MaxDimension()const noexcept;

			static JBBox Union(_In_ const JBBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept;
			static JBBox Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept;
		};
	}
}