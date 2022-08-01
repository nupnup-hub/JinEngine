#pragma once  
#include<DirectXMath.h> 
#include<type_traits> 
#include<functional> 

namespace JinEngine
{ 
	template<typename T, typename R = std::enable_if_t<std::is_integral<T>::value | std::is_floating_point<T>::value>>
	struct Vector2
	{
	public:
		T x;
		T y;
	public:
		Vector2() = default;
		Vector2(T x, T y)
			: x(x), y(y)
		{}
		Vector2(const DirectX::XMFLOAT2& xm)
			: x(xm.x), y(xm.y)
		{}
		Vector2(const Vector2&) = default;
		Vector2& operator=(const Vector2&) = default;

		Vector2(Vector2&&) = default;
		Vector2& operator=(Vector2&&) = default;

		bool operator!=(const Vector2& data) const
		{
			return x != data.x || y != data.y;
		}
		bool operator==(const Vector2& data) const
		{
			return x == data.x && y == data.y;
		}
		static Vector2 Zero() noexcept
		{
			static Vector2 zero = Vector2(0, 0);
			return  zero;
		}
		void ConvertXM(DirectX::XMFLOAT2& xmFloat)
		{
			xmFloat.x = x;
			xmFloat.y = y;
		}
		void LoadXM(DirectX::XMFLOAT2& xmFloat)
		{ 
			x = xmFloat.x;
			y = xmFloat.y;
		}
	};
	template<typename T, typename R = std::enable_if_t<std::is_integral<T>::value | std::is_floating_point<T>::value>>
	struct Vector3
	{
	public:
		T x;
		T y;
		T z;
	public:
		Vector3() = default;
		Vector3(T x, T y, T z)
			: x(x), y(y), z(z)
		{}
		Vector3(const DirectX::XMFLOAT3& xm)
			: x(xm.x), y(xm.y), z(xm.z)
		{}

		Vector3(const Vector3&) = default;
		Vector3& operator=(const Vector3&) = default;

		Vector3(Vector3&&) = default;
		Vector3& operator=(Vector3&&) = default;

		const Vector3& operator-(const Vector3& b)
		{
			return Vector3(x - b.x, y - b.y, z - b.z);
		}
		bool operator!=(const Vector3& data) const
		{
			return x != data.x || y != data.y || z != data.z;
		}
		bool operator==(const Vector3& data) const
		{
			return x == data.x && y == data.y && z == data.z;
		}
		bool operator>(const Vector3& data) const
		{
			return x > data.x && y > data.y && z > data.z;
		}
		bool operator<(const Vector3& data) const
		{
			return x < data.x&& y < data.y&& z < data.z;
		}
		static Vector3 Zero() noexcept
		{
			static Vector3<T> zero = Vector3<T>(0, 0, 0);
			return  zero;
		}
		void ConvertXM(DirectX::XMFLOAT3& xmFloat)
		{
			xmFloat.x = (float)x;
			xmFloat.y = (float)y;
			xmFloat.z = (float)z;
		}
		void LoadXM(const DirectX::XMFLOAT3& xmFloat)
		{
			x = (T)xmFloat.x;
			y = (T)xmFloat.y;
			z = (T)xmFloat.z;
		}
		void LoadXM(const DirectX::XMFLOAT4& xmFloat)
		{
			x = (T)xmFloat.x;
			y = (T)xmFloat.y;
			z = (T)xmFloat.z;
		}
	};
	template<typename T, typename R = std::enable_if_t<std::is_integral<T>::value | std::is_floating_point<T>::value>>
	struct Vector4
	{
	public:
		T x;
		T y;
		T z;
		T a;
	public:
		Vector4() = default;
		Vector4(T x, T y, T z, T a)
			: x(x), y(y), z(z), a(a)
		{}
		Vector4(const DirectX::XMFLOAT4& xm)
			: x(xm.x), y(xm.y), z(xm.z), a(xm.w)
		{}

		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;

		Vector4& operator=(const DirectX::XMFLOAT4& data)
		{
			x = data.x;
			y = data.y;
			z = data.z;
			a = data.w;
			return *this;
		}

		Vector4(Vector4&&) = default;
		Vector4& operator=(Vector4&&) = default;


		bool operator!=(const Vector4& data)
		{
			return x != data.x || y != data.y || z != data.z || a != data.a;
		}
		void ConvertXM(DirectX::XMFLOAT4& xmFloat)
		{
			xmFloat.x = x;
			xmFloat.y = y;
			xmFloat.z = z;
			xmFloat.w = a;
		}
		void LoadXM(const DirectX::XMFLOAT4& xmFloat)
		{
			x = xmFloat.x;
			y = xmFloat.y;
			z = xmFloat.z;
			a = xmFloat.w;
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
	struct hash<Vector2<T>> : public Vector2<T>
	{
		std::size_t operator()(const Vector2<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);

			return res;
		}
	};

	template <typename T>
	struct hash<Vector3<T>> : public Vector3<T>
	{
		std::size_t operator()(const Vector3<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);
			hash_combine(res, k.z);

			return res;
		}
	};

	template <typename T>
	struct hash<Vector4<T>> : public Vector4<T>
	{
		std::size_t operator()(const Vector4<T>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);
			hash_combine(res, k.z);
			hash_combine(res, k.a);
			return res;
		}
	};
}