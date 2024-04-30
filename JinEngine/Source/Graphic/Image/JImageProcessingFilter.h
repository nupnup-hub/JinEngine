#pragma once 
#include"../../Core/Math/JMatrix.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JFilter
		{
		public:
			static float Box1D(const int x, const int size);
			static float Box2D(const int x, const int y, const int size);
			static float Gaus1D(const int x, const float sig);
			static float Gaus2D(const int x, const int y, const float sig);
			static float Kaiser1D(const int x, const float beta, const int N);
			/**
			* @param beta controll sharpness
			* @param N =  kenel(row or column) count - 1
			* @brief 2-D window function => w(n1, n2) = w1(n1)w2(n2)
			*/
			static float Kaiser2D(const int x, const int y, const float beta, const int N);
		public:
			template<int calSize, int arraySize, typename ...Param >
			static void ComputeFilter(float(&kenel)[arraySize], float(*calKernel)(int, Param...), Param... param)
			{
				constexpr static int radius = calSize / 2;
				float sum = 0;
				int start = 0;
				int end = 0;
				int offset = 0;

				if (calSize % 2)
				{
					start = -radius;
					end = radius;
				}
				else
				{
					start = -radius + 1;
					end = radius;
				}
				offset = -start;

				for (int i = start; i <= end; ++i)
				{
					kenel[i + offset] = calKernel(i, param...);
					sum += kenel[i + offset];
				}
				for (int i = 0; i < calSize; ++i)
					kenel[i] /= sum;
			}
			template<int calSize, int arraySize, typename ...Param >
			static void ComputeFilter(JVector4F(&v4)[arraySize], float(*calKernel)(int, Param...), Param... param)
			{
				float kenel[calSize];
				ComputeFilter<calSize, calSize>(kenel, calKernel, param...);
				for (uint i = 0; i < calSize; ++i)
					v4[i].x = kenel[i];
			}
			template<int calSize, int arraySize, typename ...Param >
			static void Compute2DFilter(float(&kenel)[arraySize][arraySize], float(*calKernel)(int, int, Param...), Param... param)
			{
				constexpr static int radius = calSize / 2;
				float sum = 0;
				int start = 0;
				int end = 0;
				int offset = 0;

				if (calSize % 2)
				{
					start = -radius;
					end = radius;
				}
				else
				{
					start = -radius + 1;
					end = radius;
				}
				offset = -start;

				for (int i = start; i <= end; ++i)
				{
					for (int j = start; j <= end; ++j)
					{
						kenel[i + offset][j + offset] = calKernel(i, j, param...);
						sum += kenel[i + offset][j + offset];
					}
				}
				for (int i = 0; i < calSize; ++i)
				{
					for (int j = 0; j < calSize; ++j)
						kenel[i][j] /= sum;
				} 
			}		 
			template<int calSize, int arraySize, typename ...Param >
			static void Compute2DFilter(JVector4F(&v4)[arraySize], float(*calKernel)(int, int, Param...), Param... param)
			{
				float kenel[calSize][calSize];
				Compute2DFilter<calSize, calSize>(kenel, calKernel, param...);
				for (uint i = 0; i < calSize; ++i)
				{
					v4[i].x = kenel[i][0];
					if constexpr (calSize > 1)
						v4[i].y = kenel[i][1];
					if constexpr (calSize > 2)
						v4[i].z = kenel[i][2];
					if constexpr (calSize > 3)
						v4[i].w = kenel[i][3];
				}
			}
			template<int calSize, int arraySize, bool doTranspose, typename ...Param >
			static void Compute2DFilter(JMatrix4x2(&m42)[arraySize], float(*calKernel)(int, int, Param...), Param... param)
			{
				float kenel[calSize][calSize];
				Compute2DFilter<calSize, calSize>(kenel, calKernel, param...);
				for (uint i = 0; i < calSize; ++i)
				{
					if constexpr (doTranspose)
					{
						JMatrix2x4 m24;
						m24.m[0][0] = kenel[i][0];
						if constexpr (calSize > 1)
							m24.m[0][1] = kenel[i][1];
						if constexpr (calSize > 2)
							m24.m[0][2] = kenel[i][2];
						if constexpr (calSize > 3)
							m24.m[0][3] = kenel[i][3];
						if constexpr (calSize > 4)
							m24.m[1][0] = kenel[i][4];
						if constexpr (calSize > 5)
							m24.m[1][1] = kenel[i][5];
						if constexpr (calSize > 6)
							m24.m[1][2] = kenel[i][6];
						if constexpr (calSize > 7)
							m24.m[1][3] = kenel[i][7];
						m42[i] = m24.Transpose();
					}
					else
					{ 
						m42[i].m[0][0] = kenel[i][0];
						if constexpr (calSize > 1)
							m42[i].m[0][1] = kenel[i][1];
						if constexpr (calSize > 2)
							m42[i].m[1][0] = kenel[i][2];
						if constexpr (calSize > 3)
							m42[i].m[1][1] = kenel[i][3];
						if constexpr (calSize > 4)
							m42[i].m[2][0] = kenel[i][4];
						if constexpr (calSize > 5)
							m42[i].m[2][1] = kenel[i][5];
						if constexpr (calSize > 6)
							m42[i].m[3][0] = kenel[i][6];
						if constexpr (calSize > 7)
							m42[i].m[3][1] = kenel[i][7];
					} 
				}
			}
		};
	}
}