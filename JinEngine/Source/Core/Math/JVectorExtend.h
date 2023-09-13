#pragma once
#include"JVector.h"
#include<DirectXMath.h>
#include<algorithm>

namespace JinEngine
{
	//create vec2 (x, x)
	template<typename T>
	static auto CreateVec2(T x ) -> JVector2<ValidVectorParameterDetermine_T<T>>
	{
		return JVector2<ValidVectorParameterDetermine_T<T>>(x, x);
	}
	//create vec2 (x, y)
	template<typename T>
	static auto CreateVec2(T x, T y) -> JVector2<ValidVectorParameterDetermine_T<T>>
	{
		return JVector2<ValidVectorParameterDetermine_T<T>>(x, y);
	}
	//create vec3 (x, x, x)
	template<typename T>
	static auto CreateVec3(T x)-> JVector3<ValidVectorParameterDetermine_T<T>>
	{
		return JVector3<ValidVectorParameterDetermine_T<T>>(x, x, x);
	}
	//create vec3 (x, y, y)
	template<typename T>
	static auto CreateVec3(T x, T y)-> JVector3<ValidVectorParameterDetermine_T<T>>
	{
		return JVector3<ValidVectorParameterDetermine_T<T>>(x, y, y);
	}
	//create vec3 (x, y, z)
	template<typename T>
	static auto CreateVec3(T x, T y, T z)-> JVector3<ValidVectorParameterDetermine_T<T>>
	{
		return JVector3<ValidVectorParameterDetermine_T<T>>(x, y, z);
	}
	//create vec4 (x, x, x, x)
	template<typename T>
	static auto CreateVec4(T x)-> JVector4<ValidVectorParameterDetermine_T<T>>
	{
		return JVector4<ValidVectorParameterDetermine_T<T>>(x, x, x, x);
	}
	//create vec4 (x, y, y, y)
	template<typename T>
	static auto CreateVec4(T x, T y)-> JVector4<ValidVectorParameterDetermine_T<T>>
	{
		return JVector4<ValidVectorParameterDetermine_T<T>>(x, y, y, y);
	}
	//create vec4 (x, y, z, z)
	template<typename T>
	static auto CreateVec4(T x, T y, T z)-> JVector4<ValidVectorParameterDetermine_T<T>>
	{
		return JVector4<ValidVectorParameterDetermine_T<T>>(x, y, z, z);
	}
	//create vec4 (x, y, z, w)
	template<typename T>
	static auto CreateVec4(T x, T y, T z, T w)-> JVector4<ValidVectorParameterDetermine_T<T>>
	{
		return JVector4<ValidVectorParameterDetermine_T<T>>(x, y, z, w);
	}

	template<typename T>
	static JVector2<T> operator+(const DirectX::XMFLOAT2& xm, const JVector2<T>& v) noexcept
	{
		return JVector2<T>(xm.x + v.x, xm.y + v.y);
	}
	template<typename T>
	static JVector2<T> operator+(const JVector2<T>& v, const DirectX::XMFLOAT2& xm) noexcept
	{
		return JVector2<T>(v.x + xm.x, v.y + xm.y);
	}
	template<typename T>
	static JVector2<T> operator+(const JVector2<T>& v, const T f) noexcept
	{
		return JVector2<T>(v.x + f, v.y + f);
	}
	template<typename T>
	static JVector2<T> operator+(const T f, const JVector2<T>& v) noexcept
	{
		return JVector2<T>(v.x + f, v.y + f);
	}
	template<typename T>
	static JVector2<T> operator-(const DirectX::XMFLOAT2& xm, const JVector2<T>& v) noexcept
	{
		return JVector2<T>(xm.x - v.x, xm.y - v.y);
	}
	template<typename T>
	static JVector2<T> operator-(const JVector2<T>& v, const DirectX::XMFLOAT2& xm) noexcept
	{
		return JVector2<T>(v.x - xm.x, v.y - xm.y);
	}
	template<typename T>
	static JVector2<T> operator*(const JVector2<T>& v, const float f) noexcept
	{
		return JVector2<T>(v.x * f, v.y * f);
	}
	template<typename T>
	static JVector2<T> operator*(const float f, const JVector2<T>& v) noexcept
	{
		return JVector2<T>(v.x * f, v.y * f);
	} 

	template<typename T>
	static JVector3<T> operator+(const DirectX::XMFLOAT3& xm, const JVector3<T>& v) noexcept
	{
		return JVector3<T>(xm.x + v.x, xm.y + v.y, xm.z + v.z);
	}
	template<typename T>
	static JVector3<T> operator+(const JVector3<T>& v, const DirectX::XMFLOAT3& xm) noexcept
	{
		return JVector3<T>(v.x + xm.x, v.y + xm.y, v.z + xm.z);
	}
	template<typename T>
	static JVector3<T> operator+(const JVector3<T>& v, const T f) noexcept
	{
		return JVector3<T>(v.x + f, v.y + f, v.z + f);
	}
	template<typename T>
	static JVector3<T> operator+(const T f, const JVector3<T>& v) noexcept
	{
		return JVector3<T>(v.x + f, v.y + f, v.z + f);
	}
	template<typename T>
	static JVector3<T> operator-(const DirectX::XMFLOAT3& xm, const JVector3<T>& v) noexcept
	{
		return JVector3<T>(xm.x - v.x, xm.y - v.y, xm.z - v.z);
	}
	template<typename T>
	static JVector3<T> operator-(const JVector3<T>& v, const DirectX::XMFLOAT3& xm) noexcept
	{
		return JVector3<T>(v.x - xm.x, v.y - xm.y, v.z - xm.z);
	}
	template<typename T>
	static JVector3<T> operator*(const JVector3<T>& v, const float f) noexcept
	{
		return JVector3<T>(v.x * f, v.y * f, v.z * f);
	}
	template<typename T>
	static JVector3<T> operator*(const float f, const JVector3<T>& v) noexcept
	{
		return JVector3<T>(v.x * f, v.y * f, v.z * f);
	} 

	template<typename T>
	static JVector4<T> operator+(const DirectX::XMFLOAT4& xm, const JVector4<T>& v) noexcept
	{
		return JVector4<T>(xm.x + v.x, xm.y + v.y, xm.z + v.z, xm.w + v.w);
	}
	template<typename T>
	static JVector4<T> operator+(const JVector4<T>& v, const DirectX::XMFLOAT4& xm) noexcept
	{
		return JVector4<T>(v.x + xm.x, v.y + xm.y, v.z + xm.z, v.w + xm.w);
	}
	template<typename T>
	static JVector4<T> operator+(const JVector4<T>& v, const T f) noexcept
	{
		return JVector4<T>(v.x + f, v.y + f, v.z + f, v.w + f);
	}
	template<typename T>
	static JVector4<T> operator+(const T f, const JVector4<T>& v) noexcept
	{
		return JVector4<T>(v.x + f, v.y + f, v.z + f, v.w + f);
	}
	template<typename T>
	static JVector4<T> operator-(const DirectX::XMFLOAT4& xm, const JVector4<T>& v) noexcept
	{
		return JVector4<T>(xm.x - v.x, xm.y - v.y, xm.z - v.z, xm.w - v.w);
	}
	template<typename T>
	static JVector4<T> operator-(const JVector4<T>& v, const DirectX::XMFLOAT4& xm) noexcept
	{
		return JVector4<T>(v.x - xm.x, v.y - xm.y, v.z - xm.z, v.w - xm.w);
	}
	template<typename T>
	static JVector4<T> operator*(const JVector4<T>& v, const float f) noexcept
	{
		return JVector4<T>(v.x * f, v.y * f, v.z * f, v.w * f);
	}
	template<typename T>
	static JVector4<T> operator*(const float f, const JVector4<T>& v) noexcept
	{
		return JVector4<T>(v.x * f, v.y * f, v.z * f, v.w * f);
	} 


	template<typename T>
	static JVector2<T> ClampVec2(const JVector2<T>& a, const T minT, const T maxT)noexcept
	{
		return JVector2<T>(std::clamp(a.x, minT, maxT), std::clamp(a.y, minT, maxT));
	}
}