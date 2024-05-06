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
#include <DirectXMath.h> 
#include<type_traits>
#include"../JCoreEssential.h"
#include"../Math/JVector.h"
#include"../Math/JMatrix.h"

namespace JinEngine
{
	class JMathHelper
	{
	public: 
		static constexpr float Pi = 3.1415926535f;
		static constexpr float Pi2 = Pi * 2.0f;
		static constexpr float Epsilon = 1.e-6f;
		static constexpr float DegToRad = Pi / 180;
		static constexpr float RadToDeg = 180 / Pi;
	public:
		static int Log2Int(uint v)noexcept;
		static uint PowerOfTwoExponent(uint v)noexcept;
		static uint DivideTwo(uint v, int count)noexcept; 
	public: 
		// Returns simple random float in [0, 1).
		static float RandF()noexcept;
		// Returns simple random float in [a, b).
		static float RandF(float a, float b)noexcept;
		static int Rand(int a, int b)noexcept;
		// Returns the polar angle of the point (x,y) in [0, 2*PI).
		static float AngleFromXY(float x, float y)noexcept;
	public:  
		static JVector3<float> SphericalToCartesian(float radius, float theta, float phi)noexcept; 
	public:    
		static JVector4<float> Vector3BetweenQuaternion(const JVector3<float>& from, const JVector3<float>& to);
		static JVector3<float> Vector3BetweenRot(const JVector3<float>& from, const JVector3<float>& to);
		static DirectX::XMVECTOR RandUnitVec3()noexcept;
		static DirectX::XMVECTOR RandHemisphereUnitVec3(const JVector3<float>& n)noexcept;
		/**
		* @param rand = 0.0f ~ 1.0f
		*/
		static JVector2F RandomRotationTrigger(const float rand)noexcept;
	public: 
		//src A = des Get A 
		static DirectX::XMVECTOR QuaternionGap(const JVector4<float>& src, const JVector4<float>& des)noexcept;
		static JVector4<float> QuaternionInverse(const JVector4<float>& src)noexcept;
		static JVector4<float> QuaternionNormalize(const JVector4<float>& q)noexcept; 
		static DirectX::XMMATRIX QuaternionNormalize(const JMatrix4x4& m)noexcept;
		static float QuaternionNorm(const JVector4<float>& a)noexcept; 
		static JVector4<float> QuaternionZero()noexcept;
		static bool IsZeroQuaternion(const JVector4<float>& a)noexcept; 
		static DirectX::XMMATRIX ApplyQuaternion(const JMatrix4x4& m, const JVector4<float>& q)noexcept;
		static JVector4<float> PitchYawRollToQuaternion(float pitch, float yaw, float roll)noexcept;
		static JVector4<float> EulerToQuaternion(float x, float y, float z)noexcept;
		static JVector4<float> EulerToQuaternion(const JVector3<float>& e)noexcept;  
		static JVector3<float> ToEulerAngle(const JVector4<float>& q)noexcept;
		static float NormalizeAngle(float angle)noexcept;
		static JVector3<float> NormalizeAngles(const JVector3<float>& angles)noexcept; 
	public:
		static bool Contained(const JVector2F& p0, const JVector2F& p1, const JVector2F& p2, const JVector2F& v) noexcept;
	public:
		//template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Type>, int> = 0>
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static T Min(const T& a, const T& b)noexcept
		{
			return a < b ? a : b;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static T Max(const T& a, const T& b)noexcept
		{
			return a > b ? a : b;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static T Lerp(const T& a, const T& b, float t)noexcept
		{
			return a + (b - a) * t;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static T Clamp(const T& x, const T& low, const T& high)noexcept
		{
			return x < low ? low : (x > high ? high : x);
		}
		template<typename T>
		static T DivideByMultiple(T value, size_t alignment)
		{
			return (T)((value + alignment - 1) / alignment);
		}
		template<typename T>
		static T AlignT(T value, size_t alignment)
		{
			return DivideByMultiple(value, alignment) * alignment;
		}
	};
}