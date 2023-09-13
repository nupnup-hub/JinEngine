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
		static constexpr float DegToRad = Pi / 180;
		static constexpr float RadToDeg = 180 / Pi;
	public:
		static int Log2Int(uint v)noexcept;
		static uint PowerOfTwoExponent(uint v)noexcept;
		static uint DivideTwo(uint v, int count)noexcept;
	public:
		static float Floor(const float value, const uint decimalPlace);
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
	};
}