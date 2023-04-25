#pragma once
#include"JVector.h"
#include<DirectXMath.h>

namespace JinEngine
{
	template<typename T>
	static auto CreateVec2(T x) -> JVector2<ValidVectorParameterDetermine_T<T>>
	{
		return JVector2<ValidVectorParameterDetermine_T<T>>(x, x);
	}
	template<typename T>
	static auto CreateVec3(T x)-> JVector3<ValidVectorParameterDetermine_T<T>>
	{
		return JVector3<ValidVectorParameterDetermine_T<T>>(x, x, x);
	}
	template<typename T>
	static auto CreateVec4(T x)-> JVector4<ValidVectorParameterDetermine_T<T>>
	{
		return JVector4<ValidVectorParameterDetermine_T<T>>(x, x, x, x);
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
	static JVector2<T> operator-(const DirectX::XMFLOAT2& xm, const JVector2<T>& v) noexcept
	{
		return JVector2<T>(xm.x - v.x, xm.y - v.y);
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
	static JVector3<T> operator-(const DirectX::XMFLOAT3& xm, const JVector3<T>& v) noexcept
	{
		return JVector3<T>(xm.x - v.x, xm.y - v.y, xm.z - v.z);
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
	static JVector4<T> operator-(const DirectX::XMFLOAT4& xm, const JVector3<T>& v) noexcept
	{
		return JVector4<T>(xm.x - v.x, xm.y - v.y, xm.z - v.z, xm.w - v.w);
	}
}