#pragma once
#include"../JDataType.h"
#include"../../Utility/JVector.h"
#include<DirectXMath.h>
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Core
	{
		class JBBox2D
		{
		public:
			JVector2<float> min;
			JVector2<float> max;
		public: 
			JBBox2D(const JVector2<float>& min, const JVector2<float>& max);
		public:
			bool Contain(const JVector2<float>& p);
			JVector2<float> Extent()const noexcept;
			JVector2<float> Center()const noexcept;
		};

		class JBBox
		{
		public:
			//DirectX::XMFLOAT3 min;
			//DirectX::XMFLOAT3 max;
		public:
			JVector3<float> min;
			JVector3<float> max;
		public:
			JBBox(const JVector3<float>& min = JVector3<float>(0,0,0), const JVector3<float>& max = JVector3<float>(0, 0, 0));
			JBBox(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max);
			JBBox(const DirectX::BoundingBox& boundBox);		 
			~JBBox();
			JBBox(const JBBox& rhs) = default;
			JBBox& operator=(const JBBox& rhs) = default;
			JBBox(JBBox&& rhs) = default;
			JBBox& operator=(JBBox&& rhs) = default;
		public:
			DirectX::BoundingBox Convert()const noexcept;
			JVector3<float> Extent()const noexcept;
			JVector3<float> Center()const noexcept;
			JVector3<float> DistanceVector()const noexcept;
			float Surface()const noexcept;
			/*
				Dimension
				x = 0, y = 1, z = 2
			*/
			uint MaxDimension()const noexcept;
		public:
			static JBBox Union(_In_ const JBBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept;
			static JBBox Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept;
			static JBBox InfBBox()noexcept;
		};
	}
}