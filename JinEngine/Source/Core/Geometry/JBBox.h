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
#include"../JCoreEssential.h"
#include"../Math/JVector.h"
#include<DirectXMath.h>
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Core
	{
		class JRay2D;
		class JBBox2D
		{
		public:
			JVector2<float> minP;
			JVector2<float> maxP;
		public:  
			JBBox2D(const JVector2<float>& minP = JVector2F::Zero(), const JVector2<float>& maxP = JVector2F::Zero());
		public:
			bool Contain(const JVector2<float>& p)const noexcept; 
			bool Intersect(const JBBox2D& b)const noexcept;
			bool Intersect(const JRay2D& ray, JVector2<float>* hitPoint = nullptr)const noexcept; 
			JVector2<float> Extent()const noexcept;
			JVector2<float> Center()const noexcept;
			JVector2<float> DistanceVector()const noexcept;
		};

		class JBBox
		{ 
		public:
			JVector3<float> minP;
			JVector3<float> maxP;
		public:
			JBBox(const JVector3<float>& minP = JVector3<float>(0,0,0), const JVector3<float>& maxP = JVector3<float>(0, 0, 0));
			JBBox(const DirectX::XMFLOAT3& minP, const DirectX::XMFLOAT3& maxP);
			JBBox(const DirectX::BoundingBox& boundBox);		 
			~JBBox();
			JBBox(const JBBox& rhs) = default;
			JBBox& operator=(const JBBox& rhs) = default;
			JBBox(JBBox&& rhs) = default;
			JBBox& operator=(JBBox&& rhs) = default;
		public:
			DirectX::BoundingBox Convert()const noexcept;
			void Coners(_Out_ JVector3<float>(&outCorners)[8])const noexcept;
			JVector3<float> Extent()const noexcept;
			JVector3<float> Center()const noexcept;
			JVector3<float> DistanceVector()const noexcept;
			/*
				Dimension
				x = 0, y = 1, z = 2
			*/
			float DistancePoint(const uint p)const noexcept;
			float Surface()const noexcept;
			/*
				Dimension
				x = 0, y = 1, z = 2
			*/
			uint MaxDimension()const noexcept;
		public:
			void Lengthen(const JVector3<float>& length);
		public:
			bool IsDistanceZero()const noexcept;
		public:
			static JBBox Union(_In_ const JBBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept;
			static JBBox Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept;
			static JBBox InfBBox()noexcept;
		};
	}
}