#pragma once
#include"JVector.h"
#include<DirectXMath.h>

namespace JinEngine
{
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
}