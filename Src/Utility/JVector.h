#pragma once  
#include<DirectXMath.h> 
#include<type_traits> 
#include<functional> 
 
namespace JinEngine
{ 
	template<typename T>
	struct JVector2
	{
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
		JVector2(const JVector2&) = default;
		JVector2& operator=(const JVector2& lhs) = default;
 
		JVector2(JVector2&&) = default;
		JVector2& operator=(JVector2&&) = default;

		bool operator!=(const JVector2& data) const
		{
			return x != data.x || y != data.y;
		}
		bool operator==(const JVector2& data) const
		{
			return x == data.x && y == data.y;
		}
		static JVector2 Zero() noexcept
		{
			static JVector2 zero = JVector2(0, 0);
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
		bool Contain(const JVector2& pos, const JVector2& size)
		{
			return x >= pos.x && x <= pos.x + size.x && y >= pos.y && y <= pos.y + size.y;
		}
	};
	template<typename T>
	struct JVector3
	{
	public:
		T x;
		T y;
		T z;
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

		const JVector3& operator-(const JVector3& b)
		{
			return JVector3(x - b.x, y - b.y, z - b.z);
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
		static JVector3 Zero() noexcept
		{
			static JVector3<T> zero = JVector3<T>(0, 0, 0);
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
	template<typename T>
	struct JVector4
	{
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

		JVector4(const JVector4&) = default;
		JVector4& operator=(const JVector4&) = default;
		JVector4& operator=(const DirectX::XMFLOAT4& data)
		{
			x = data.x;
			y = data.y;
			z = data.z;
			w= data.w;
			return *this;
		}

		JVector4(JVector4&&) = default;
		JVector4& operator=(JVector4&&) = default;


		bool operator!=(const JVector4& data)
		{
			return x != data.x || y != data.y || z != data.z || w != data.w;
		}
		void ConvertXM(DirectX::XMFLOAT4& xmFloat)
		{
			xmFloat.x = x;
			xmFloat.y = y;
			xmFloat.z = z;
			xmFloat.w = w;
		}
		void LoadXM(const DirectX::XMFLOAT4& xmFloat)
		{
			x = xmFloat.x;
			y = xmFloat.y;
			z = xmFloat.z;
			w = xmFloat.w;
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