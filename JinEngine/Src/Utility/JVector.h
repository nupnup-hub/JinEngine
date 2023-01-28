#pragma once  
#include<DirectXMath.h> 
#include<type_traits> 
#include<functional>  
#include"../Core/JDataType.h"

struct ImVec2;
struct ImVec4;

namespace JinEngine
{ 
	struct JVectorBase{};
	template<typename T>
	struct JVector2 : public JVectorBase
	{ 
	public:
		using ValueType = T;
	public:
		T x;
		T y;
	public:
		JVector2() = default;
		JVector2(T x, T y)
			: x(x), y(y)
		{}
		JVector2(const DirectX::XMFLOAT2& xm)
			: x(xm.x), y(xm.y)
		{} 
		JVector2(const ImVec2& v);
		JVector2(const JVector2&) = default;
		JVector2& operator=(const JVector2& rhs) = default;
		JVector2& operator=(const ImVec2& rhs);
		JVector2(JVector2&&) = default;
		JVector2& operator=(JVector2&&) = default;
	public:
		template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
		JVector2(const JVector2<U>& rhs)
		{
			x = rhs.x;
			y = rhs.y;
		}
		template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
		JVector2& operator=(const JVector2<U>& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}
	public:
		JVector2 operator+(const JVector2& rhs)const
		{
			return JVector2(x + rhs.x, y + rhs.y);
		}
		JVector2 operator-(const JVector2& rhs)const
		{
			return JVector2(x - rhs.x, y - rhs.y);
		}
		JVector2 operator*(float rhs)const
		{
			return JVector2(x * rhs, y * rhs);
		}
		JVector2 operator/(float rhs)const
		{
			return JVector2(x / rhs, y / rhs);
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
		}
		void operator/=(float rhs)
		{
			x /= rhs;
			y /= rhs; 
		}
		bool operator!=(const JVector2& data) const
		{
			return x != data.x || y != data.y;
		}
		bool operator==(const JVector2& data) const
		{
			return x == data.x && y == data.y;
		}
		T& operator[](const uint index)
		{
			if (index == 0)
				return x;
			else if (index == 1)
				return y; 
			else
			{
				assert("JVector3 operator index error");
				return x;
			}
		}
	public:
		static JVector2 Zero()noexcept
		{ 
			return JVector2(0, 0);
		}
		DirectX::XMFLOAT2 ConvertXMF()const noexcept
		{
			return DirectX::XMFLOAT2{ (float)x, float(y) };
		}
		bool Contain(const JVector2& pos, const JVector2& size)const noexcept
		{
			return x >= pos.x && x <= pos.x + size.x && y >= pos.y && y <= pos.y + size.y;
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 2;
		}
	};


	template<typename T>
	struct JVector3 : public JVectorBase
	{
	public:
		using ValueType = T;
	public:
		mutable T x;
		mutable T y;
		mutable T z;
	public:
		JVector3() = default;
		JVector3(T x, T y, T z)
			: x(x), y(y), z(z)
		{}
		JVector3(const DirectX::XMFLOAT3& xm)
			: x(xm.x), y(xm.y), z(xm.z)
		{}

		JVector3(const JVector3&) = default;
		JVector3& operator=(const JVector3&) = default;

		JVector3(JVector3&&) = default;
		JVector3& operator=(JVector3&&) = default;
	public:
		JVector3 operator+(const JVector3& rhs)const
		{
			return JVector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		JVector3 operator-(const JVector3& rhs)const
		{
			return JVector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}
		JVector3 operator*(float rhs)const
		{
			return JVector3(x * rhs, y * rhs, z * rhs);
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
		}
		void operator/=(float rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
		}
		bool operator!=(const JVector3& data) const
		{
			return x != data.x || y != data.y || z != data.z;
		}
		bool operator==(const JVector3& data) const
		{
			return x == data.x && y == data.y && z == data.z;
		}
		bool operator>(const JVector3& data) const
		{
			return x > data.x && y > data.y && z > data.z;
		}
		bool operator<(const JVector3& data) const
		{
			return x < data.x&& y < data.y&& z < data.z;
		}
		T& operator[](const uint index) const
		{
			if (index == 0)
				return x;
			else if (index == 1)
				return y;
			else if (index == 2)
				return z;
			else
			{
				assert("JVector3 operator index error");
				return x;
			}
		}
	public:
		static JVector3 Zero() noexcept
		{ 
			return JVector3(0, 0, 0);
		}
		DirectX::XMFLOAT3 ConvertXMF()const noexcept
		{
			return DirectX::XMFLOAT3{(float)x, (float)y, (float)z };
		}
		DirectX::XMVECTOR ConvertXMV()const noexcept
		{
			return DirectX::XMVectorSet(x, y, z, 1.0f);
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 3;
		}
	};

	template<typename T>
	struct JVector4 : public JVectorBase
	{
	public:
		using ValueType = T;
	public:
		T x;
		T y;
		T z;
		T w;
	public:
		JVector4() = default; 
		JVector4(T x, T y, T z, T w)
			: x(x), y(y), z(z), w(w)
		{}
		JVector4(const DirectX::XMFLOAT4& xm)
			: x(xm.x), y(xm.y), z(xm.z), w(xm.w)
		{}
		JVector4(const ImVec4& v);

		JVector4(const JVector4&) = default;
		JVector4& operator=(const JVector4&) = default;
		JVector4& operator=(const DirectX::XMFLOAT4& data)
		{
			x = data.x;
			y = data.y;
			z = data.z;
			w = data.w;
			return *this;
		}
		JVector4& operator=(const ImVec4& rhs);

		JVector4(JVector4&&) = default;
		JVector4& operator=(JVector4&&) = default;
	public:
		JVector4 operator+(const JVector4& rhs)const
		{
			return JVector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}
		JVector4 operator-(const JVector4& rhs)const
		{
			return JVector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}
		JVector4 operator*(float rhs)const
		{
			return JVector4(x * rhs, y * rhs, z * rhs, w * rhs);
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
		}
		void operator/=(float rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			w /= rhs;
		}
		bool operator!=(const JVector4& data)
		{
			return x != data.x || y != data.y || z != data.z || w != data.w;
		}
		T& operator[](const uint index)
		{
			if (index == 0)
				return x;
			else if (index == 1)
				return y;
			else if (index == 2)
				return z;
			else if (index == 3)
				return w;
			else
			{
				assert("JVector3 operator index error");
				return x;
			}
		}
	public:
		DirectX::XMFLOAT4 ConvertXMF()const noexcept
		{
			return DirectX::XMFLOAT4{(float) x,(float)y, (float)z, (float)w };
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 4;
		}
	};

}


namespace std
{
	using namespace JinEngine;

	template <class T>
	static inline void hash_combine(std::size_t& s, const T& v)
	{
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	} 
	template <typename T>
	struct hash<JVector2<T>> : public JVector2<T>
	{
		std::size_t operator()(const JVector2<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);

			return res;
		}
	};

	template <typename T>
	struct hash<JVector3<T>> : public JVector3<T>
	{
		std::size_t operator()(const JVector3<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);
			hash_combine(res, k.z);

			return res;
		}
	};

	template <typename T>
	struct hash<JVector4<T>> : public JVector4<T>
	{
		std::size_t operator()(const JVector4<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);
			hash_combine(res, k.z);
			hash_combine(res, k.w);
			return res;
		}
	};
}