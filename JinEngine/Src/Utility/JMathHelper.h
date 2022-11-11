#pragma once
#include <DirectXMath.h> 
#include<type_traits>
#include"../Core/JDataType.h"
#include"../Utility/JVector.h"

namespace JinEngine
{
	class JMathHelper
	{
	public:
		static constexpr float Infinity = FLT_MAX;
		static constexpr float Pi = 3.1415926535f;
		static constexpr float DegToRad = Pi / 180;
		static constexpr float RadToDeg = 180 / Pi;
	public:
		static int Log2Int(uint v)noexcept;
	public:
		// Returns random float in [0, 1).
		static float RandF()noexcept;
		// Returns random float in [a, b).
		static float RandF(float a, float b)noexcept;
		static int Rand(int a, int b)noexcept;
		// Returns the polar angle of the point (x,y) in [0, 2*PI).
		static float AngleFromXY(float x, float y)noexcept;
	public:
		static DirectX::XMVECTOR VectorLeft()noexcept;
		static DirectX::XMVECTOR VectorRight()noexcept;
		static DirectX::XMVECTOR VectorUp()noexcept;
		static DirectX::XMVECTOR VectorDown()noexcept;
		static DirectX::XMVECTOR VectorForward()noexcept;
		static DirectX::XMVECTOR VectorBack()noexcept;
		static DirectX::XMVECTOR VectorBasicPoint()noexcept;
	public:
		static DirectX::XMFLOAT3 Min3F()noexcept;
		static DirectX::XMFLOAT3 Max3F()noexcept;
		static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)noexcept;
		static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)noexcept;
		static DirectX::XMFLOAT2 Zero2f()noexcept;
		static DirectX::XMFLOAT3 Zero3f()noexcept;
		static DirectX::XMFLOAT4 Zero4f()noexcept;
		static DirectX::XMFLOAT4X4 Identity4x4()noexcept;
		static DirectX::XMMATRIX IdentityMatrix4()noexcept;
	public:
		static DirectX::XMFLOAT3 Vector3Multiply(const DirectX::XMFLOAT3& src, float s)noexcept;
		static DirectX::XMFLOAT3 Vector3Multiply(const DirectX::XMFLOAT3& src, const DirectX::XMFLOAT4X4& m)noexcept;
		static DirectX::XMFLOAT4 Vector4Multiply(const DirectX::XMFLOAT4& src, const DirectX::XMFLOAT4X4& m)noexcept;
		static float VectorDot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept;
		static float VectorDot(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept;
		static DirectX::XMFLOAT3 Vector3Cross(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept;
		static DirectX::XMFLOAT3 Vector4Cross(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept;
		static DirectX::XMFLOAT3 Vector3Plus(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept;
		static DirectX::XMFLOAT4 Vector4Plus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept;
		static DirectX::XMFLOAT3 Vector3Normalize(const DirectX::XMFLOAT3& src)noexcept;
		static DirectX::XMFLOAT4 Vector4Normalize(const DirectX::XMFLOAT4& src)noexcept;
		static float Vector3Length(const DirectX::XMFLOAT3& src)noexcept;
		static float Vector3Length(const JVector3<float>& src )noexcept;
		static float Vector4Length(const DirectX::XMFLOAT4& src)noexcept;
		static DirectX::XMFLOAT3 Vector3Minus(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept;
		static DirectX::XMFLOAT4 Vector4Minus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept;
		static DirectX::XMVECTOR RandUnitVec3()noexcept;
		static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n)noexcept;
	public:
		static DirectX::XMFLOAT4 QuaternionPlus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept; 
		//src A = des Get A
		static DirectX::XMVECTOR QuaternionGap(const DirectX::XMVECTOR src, const DirectX::XMVECTOR des)noexcept;
		static DirectX::XMVECTOR QuaternionGap(const DirectX::XMFLOAT4& src, const DirectX::XMFLOAT4& des)noexcept;
		static DirectX::XMFLOAT4 QuaternionInverse(const DirectX::XMFLOAT4& src)noexcept;
		static DirectX::XMFLOAT4 QuaternionNormalize(const DirectX::XMFLOAT4& q)noexcept;
		static void QuaternionNormalize(DirectX::XMFLOAT4X4& m)noexcept;
		static DirectX::XMMATRIX QuaternionNormalize(DirectX::XMMATRIX m)noexcept;
		static float QuaternionNorm(const DirectX::XMFLOAT4& a)noexcept; 
		static DirectX::XMFLOAT4 QuaternionZero()noexcept;
		static bool IsZeroQuaternion(const DirectX::XMFLOAT4& a)noexcept;
		static DirectX::XMMATRIX ApplyQuaternion(DirectX::FXMMATRIX m, const DirectX::XMFLOAT4& q)noexcept;
		static DirectX::XMFLOAT4 PitchYawRollToQuaternion(float pitch, float yaw, float roll)noexcept;
		static DirectX::XMFLOAT4 EulerToQuaternion(float x, float y, float z)noexcept;
		static DirectX::XMFLOAT3 ToEulerAngle(const DirectX::XMVECTOR q)noexcept;
		static DirectX::XMFLOAT3 ToEulerAngle(const DirectX::XMFLOAT4& q)noexcept;
		static float NormalizeAngle(float angle)noexcept;
		static DirectX::XMFLOAT3 NormalizeAngles(const DirectX::XMFLOAT3& angles)noexcept;
	public:
		static bool IsSame(const DirectX::XMFLOAT4X4& a, const DirectX::XMFLOAT4X4& b)noexcept;
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