#pragma once  
#include<DirectXMath.h> 
#include<type_traits> 
#include<functional>   
#include"../Core/JDataType.h"
 
struct ImVec2;
struct ImVec4;
 
namespace JinEngine
{
	template<typename T>
	struct ValidVectorParameterDetermine
	{
		using Type = std::conditional_t<std::is_floating_point_v<T> || std::is_integral_v<T>, T, void>;
	}; 
	template<typename T>
	using ValidVectorParameterDetermine_T = typename ValidVectorParameterDetermine<T>::Type;

	struct JVectorBase {};
	template<typename T>
	struct JVector2 : public JVectorBase
	{
	public:
		using ValueType = typename ValidVectorParameterDetermine_T<T>;
	public:
		ValueType x;
		ValueType y;
	public:
		JVector2() = default; 
		JVector2(ValueType x, ValueType y)
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
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector2(const JVector2<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
		}
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector2& operator=(const JVector2<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
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
		JVector2 operator*(const JVector2& rhs)const
		{
			return JVector2(x * rhs.x, y * rhs.y);
		}
		JVector2 operator*(float rhs)const
		{
			return JVector2(static_cast<ValueType>(x * rhs), static_cast<ValueType>(y * rhs));
		}
		JVector2 operator/(const JVector2& rhs)const
		{
			return JVector2(x / rhs.x, y / rhs.y);
		}
		JVector2 operator/(float rhs)const
		{
			return JVector2(static_cast<ValueType>(x / rhs), static_cast<ValueType>(y / rhs)); 
		}
		void operator+=(const JVector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
		}
		void operator-=(const JVector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
		}
		void operator*=(const JVector2& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
		}
		void operator/=(const JVector2& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
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
		ValueType& operator[](const uint index)
		{
			if (index == 0)
				return x;
			else if (index == 1)
				return y;
			else
			{
				assert("JVector2 operator index error");
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
		using ValueType = typename ValidVectorParameterDetermine_T<T>;
	public:
		mutable ValueType x;
		mutable ValueType y;
		mutable ValueType z;
	public:
		JVector3() = default; 
		JVector3(ValueType x, ValueType y, ValueType z)
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
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector3(const JVector3<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
		}
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector3& operator=(const JVector3<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
			return *this;
		}
	public:
		JVector3 operator+(const JVector3& rhs)const
		{
			return JVector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		JVector3 operator-(const JVector3& rhs)const
		{
			return JVector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}
		JVector3 operator*(const JVector3& rhs)const
		{
			return JVector3(x * rhs.x, y * rhs.y, z * rhs.z);
		}
		JVector3 operator*(float rhs)const
		{
			return JVector3(static_cast<ValueType>(x * rhs), static_cast<ValueType>(y * rhs), static_cast<ValueType>(z * rhs));
		}
		JVector3 operator/(const JVector3& rhs)const
		{
			return JVector3(x / rhs.x, y / rhs.y, z / rhs.z);
		}
		JVector3 operator/(float rhs)const
		{
			return JVector3(static_cast<ValueType>(x / rhs), static_cast<ValueType>(y / rhs), static_cast<ValueType>(z / rhs));
		}
		void operator+=(const JVector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
		}
		void operator-=(const JVector3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
		}
		void operator*=(const JVector3& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
		}
		void operator/=(const JVector3& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
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
		ValueType& operator[](const uint index) const
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
			return DirectX::XMFLOAT3{ (float)x, (float)y, (float)z };
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
		using ValueType = typename ValidVectorParameterDetermine_T<T>;
	public:
		ValueType x;
		ValueType y;
		ValueType z;
		ValueType w;
	public:
		JVector4() = default; 
		JVector4(ValueType x, ValueType y, ValueType z, ValueType w)
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
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector4(const JVector4<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
			w = static_cast<ValueType>(rhs.w);
		}
		template<typename U, std::enable_if_t<std::is_convertible_v<ValueType, U>, int> = 0>
		JVector4& operator=(const JVector4<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
			w = static_cast<ValueType>(rhs.w);
			return *this;
		}
	public:
		JVector4 operator+(const JVector4& rhs)const
		{
			return JVector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}
		JVector4 operator-(const JVector4& rhs)const
		{
			return JVector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}
		JVector4 operator*(const JVector4& rhs)const
		{
			return JVector4(x * rhs, y * rhs, z * rhs, w * rhs);
		}
		JVector4 operator*(float rhs)const
		{
			return JVector4(static_cast<ValueType>(x * rhs), static_cast<ValueType>(y * rhs), static_cast<ValueType>(z * rhs), static_cast<ValueType>(w * rhs));
		}
		JVector4 operator/(const JVector4& rhs)const
		{
			return JVector4(x / rhs, y / rhs, z / rhs, w / rhs);
		}
		JVector4 operator/(float rhs)const
		{
			return JVector4(static_cast<ValueType>(x / rhs), static_cast<ValueType>(y / rhs), static_cast<ValueType>(z / rhs), static_cast<ValueType>(w / rhs));
		}
		void operator+=(const JVector4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
		}
		void operator-=(const JVector4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
		}
		void operator*=(const JVector4& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;
		}
		void operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
		}
		void operator/=(const JVector4& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;
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
		ValueType& operator[](const uint index)
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
				assert("JVector4 operator index error");
				return x;
			}
		}
	public:
		DirectX::XMFLOAT4 ConvertXMF()const noexcept
		{
			return DirectX::XMFLOAT4{ (float)x,(float)y, (float)z, (float)w };
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 4;
		}
	};

	template<>
	struct JVector2<void>;
	template<>
	struct JVector3<void>;
	template<>
	struct JVector4<void>;
}


namespace std
{
	using namespace JinEngine;

	template <class ValueType>
	static inline void hash_combine(std::size_t& s, const ValueType& v)
	{
		std::hash<ValueType> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}
	template <typename ValueType>
	struct hash<JVector2<ValueType>> : public JVector2<ValueType>
	{
		std::size_t operator()(const JVector2<ValueType>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);

			return res;
		}
	};

	template <typename ValueType>
	struct hash<JVector3<ValueType>> : public JVector3<ValueType>
	{
		std::size_t operator()(const JVector3<ValueType>& k) const
		{
			std::size_t res = 0;
			hash_combine(res, k.x);
			hash_combine(res, k.y);
			hash_combine(res, k.z);

			return res;
		}
	};

	template <typename ValueType>
	struct hash<JVector4<ValueType>> : public JVector4<ValueType>
	{
		std::size_t operator()(const JVector4<ValueType>& k) const
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