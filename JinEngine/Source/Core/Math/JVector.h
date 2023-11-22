#pragma once  
#include<DirectXMath.h> 
#include<type_traits> 
#include<functional>   
#include <cmath>
#include<string>
#include<algorithm>
#include"../JCoreEssential.h"

namespace JinEngine
{
	template<typename T>
	struct ValidVectorParameterDetermine
	{
		using Type = std::conditional_t<std::is_floating_point_v<T> || std::is_integral_v<T>, T, void>;
	};
	template<typename T>
	using ValidVectorParameterDetermine_T = typename ValidVectorParameterDetermine<T>::Type;

	struct JVectorBase
	{
	public:
		template<typename T>
		static constexpr T PositiveInf()
		{
			if constexpr (std::is_same_v<float, T>)
				return FLT_MAX;
			else if constexpr (std::is_same_v<double, T>)
				return DBL_MAX;
			else if constexpr (std::is_same_v<short, T>)
				return SHRT_MAX;
			else if constexpr (std::is_same_v<ushort, T>)
				return USHRT_MAX;
			else if constexpr (std::is_same_v<int, T>)
				return INT_MAX;
			else if constexpr (std::is_same_v<uint, T>)
				return UINT_MAX;
			else if constexpr (std::is_same_v<size_t, T>)
				return SIZE_MAX;
			else if constexpr (std::is_same_v<int8, T>)
				return INT8_MAX;
			else if constexpr (std::is_same_v<uint8, T>)
				return UINT8_MAX;
			else if constexpr (std::is_same_v<int16, T>)
				return INT16_MAX;
			else if constexpr (std::is_same_v<uint16, T>)
				return UINT16_MAX;
			else if constexpr (std::is_same_v<int32, T>)
				return INT32_MAX;
			else if constexpr (std::is_same_v<uint32, T>)
				return UINT32_MAX;
			else if constexpr (std::is_same_v<int64, T>)
				return INT64_MAX;
			else if constexpr (std::is_same_v<uint64, T>)
				return UINT64_MAX;
			else
				return 0;
		}
		template<typename T>
		static constexpr T NegativeInf()
		{
			if constexpr (std::is_same_v<float, T>)
				return -FLT_MAX;
			else if constexpr (std::is_same_v<double, T>)
				return -DBL_MAX;
			else if constexpr (std::is_same_v<short, T>)
				return -SHRT_MAX;
			else if constexpr (std::is_same_v<int, T>)
				return -INT_MAX;
			else if constexpr (std::is_same_v<int8, T>)
				return -INT8_MAX;
			else if constexpr (std::is_same_v<int16, T>)
				return -INT16_MAX;
			else if constexpr (std::is_same_v<int32, T>)
				return -INT32_MAX;
			else if constexpr (std::is_same_v<int64, T>)
				return -INT64_MAX;
			else
				return 0;
		}
	};
	template<typename T>
	struct JVector2 : public JVectorBase
	{
	public:
		using ValueType = typename ValidVectorParameterDetermine_T<T>;
		using Base = JVectorBase;
	private:
		template<typename S, typename U = void>
		struct HasXY
		{
		public:
			static constexpr bool value = false;
		};
		template<typename S>
		struct HasXY<S, std::void_t<decltype(&S::x), decltype(&S::y)>>
		{
		public:
			static constexpr bool value = sizeof(S) == sizeof(JVector2<T>) &&
				std::is_convertible_v<decltype(S::x), ValueType> &&
				std::is_convertible_v<decltype(S::y), ValueType>;
		};
	public:
		static constexpr bool isValidValue = !std::is_same_v<ValueType, void>;
	public:
		ValueType x;
		ValueType y;
	public:
		JVector2()
		{
			if constexpr (std::is_integral_v< ValueType> || std::is_floating_point_v<ValueType>)
				x = y = 0;
			else if constexpr (std::is_enum_v<ValueType>)
				x = y = (ValueType)0;
		}
		JVector2(const ValueType x, const ValueType y)
			: x(x), y(y)
		{}
#if defined(_XM_NO_INTRINSICS_) 
		JVector2(const __vector4& v)
			: x(v.vector4_f32[0]), y(v.vector4_f32[1]))
		{}
#elif defined(_XM_ARM_NEON_INTRINSICS_) 
		JVector2(const float32x4_t& v)
			: x(vgetq_lane_f32(v, 0)), y(vgetq_lane_f32(v, 1))
		{}
#elif defined(_XM_SSE_INTRINSICS_) 
		JVector2(const __m128& v)
		{
			_mm_store_ss(&x, v);
			_mm_store_ss(&y, XM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1))); 
		}
#endif 
		JVector2(const JVector2&) = default;
		JVector2& operator=(const JVector2& rhs) = default;
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
		template<typename U, std::enable_if_t<HasXY<U>::value, int> = 0>
		JVector2(const U& v)
			: x(v.x), y(v.y)
		{}
		template<typename U, std::enable_if_t<HasXY<U>::value, int> = 0>
		JVector2& operator=(const U& v)
		{
			x = static_cast<ValueType>(v.x);
			y = static_cast<ValueType>(v.y);
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
			return x != data.x && y != data.y;
		}
		bool operator==(const JVector2& data) const
		{
			return x == data.x && y == data.y;
		}
		ValueType& operator[](const uint index)noexcept
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
		const ValueType& operator[](const uint index)const noexcept
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
		ValueType* Data()noexcept
		{
			return &x;
		}
	public:
		template<typename T1, typename T2>
		static float Dot(const JVector2<T1>& v1, const JVector2<T2>& v2) noexcept
		{
			return (float)(v1.x * v2.x + v1.y * v2.y);
		}
		template<typename T1, typename T2>
		static JVector2 Cross(const JVector2<T1>& v1, const JVector2<T2>& v2) noexcept
		{
			return DirectX::XMVector2Cross(v1.ToXmV(), v2.ToXmV());
		}
		static JVector2 Min(const JVector2& a, const JVector2& b)noexcept
		{
			return JVector2(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
		}
		static JVector2 Max(const JVector2& a, const JVector2& b)noexcept
		{
			return JVector2(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
		}
		void Clamp(const JVector2& value, const T minV, const T maxV)noexcept
		{
			x = std::clamp(value.x, minV, maxV);
			y = std::clamp(value.y, minV, maxV);
		}
		void Clamp(const JVector2& value, const JVector2& minV, const JVector2& maxV)noexcept
		{
			x = std::clamp(value.x, minV.x, maxV.x);
			y = std::clamp(value.y, minV.y, maxV.y);
		}
	public:
		float Length()const noexcept
		{
			return abs(sqrt(x * x + y * y));
		}
		JVector2 Normalize()const noexcept
		{
			return DirectX::XMVector2Normalize(ToXmV());
		}
		bool Contained(const JVector2& pos, const JVector2& size)const noexcept
		{
			return x >= pos.x && x <= (pos.x + size.x) && y >= pos.y && y <= (pos.y + size.y);
		}
	public:
		static JVector2 Zero() noexcept
		{
			return JVector2(0, 0);
		}
		static JVector2 One()noexcept
		{
			return JVector2(1, 1);
		}
		static JVector2 PositiveInfV()noexcept
		{
			return JVector2(Base::PositiveInf<T>(), Base::PositiveInf<T>());
		}
		static JVector2 NegativeInfV()noexcept
		{
			return JVector2(Base::NegativeInf<T>(), Base::NegativeInf<T>());
		}
	public:
		template<typename T>
		auto ToSimilar()const noexcept -> std::conditional_t<HasXY<T>::value, T, void>
		{
			return T{ (float)x, (float)y };
		}
		DirectX::XMVECTOR ToXmV(const float w = 1.0f)const noexcept
		{
			if constexpr (std::is_same_v<ValueType, T>)
				return DirectX::XMVectorSet(x, y, 0.0f, w);
			else
				return DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, w);
		}
		std::string ToString()const noexcept
		{
			return std::to_string(x) + " " + std::to_string(y);
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
		using Base = JVectorBase;
	private:
		template<typename S, typename U = void>
		struct HasXYZ
		{
		public:
			static constexpr bool value = false;
		};
		template<typename S>
		struct HasXYZ<S, std::void_t<decltype(&S::x), decltype(&S::y), decltype(&S::z)>>
		{
		public:
			static constexpr bool value = sizeof(S) == sizeof(JVector3<T>) &&
				std::is_convertible_v<decltype(S::x), ValueType> &&
				std::is_convertible_v<decltype(S::y), ValueType> &&
				std::is_convertible_v<decltype(S::z), ValueType>;
		};
	public:
		static constexpr bool isValidValue = !std::is_same_v<ValueType, void>;
	public:
		ValueType x;
		ValueType y;
		ValueType z;
	public:
		JVector3()
		{
			if constexpr (std::is_integral_v< ValueType> || std::is_floating_point_v<ValueType>)
				x = y = z = 0;
			else if constexpr (std::is_enum_v<ValueType>)
				x = y = z = (ValueType)0;
		}
		JVector3(const ValueType x, const ValueType y, const ValueType z)
			: x(x), y(y), z(z)
		{}
		JVector3(const ValueType(&v)[3])
			: x(v[0]), y(v[1]), z(v[2])
		{}
		JVector3(const DirectX::XMFLOAT3& xm)
			: x(xm.x), y(xm.y), z(xm.z)
		{}
#if defined(_XM_NO_INTRINSICS_) 
		JVector3(const __vector4& v)
			: x(v.vector4_f32[0]), y(v.vector4_f32[1])), z(v.vector4_f32[2]))
		{}
#elif defined(_XM_ARM_NEON_INTRINSICS_) 
		JVector3(const float32x4_t& v)
			: x(vgetq_lane_f32(v, 0)), y(vgetq_lane_f32(v, 1)), z(vgetq_lane_f32(v, 2))
		{}
#elif defined(_XM_SSE_INTRINSICS_) 
		JVector3(const __m128& v)
		{
			_mm_store_ss(&x, v);
			_mm_store_ss(&y, XM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1)));
			_mm_store_ss(&z, XM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2))); 
		}
#endif 
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
		template<typename U>
		JVector3(const JVector2<U>& v2, const typename JVector2<U>::ValueType z1)
			: x(v2.x), y(v2.y), z(z1)
		{}
		template<typename U, std::enable_if_t<HasXYZ<U>::value, int> = 0>
		JVector3(const U& v)
			: x(v.x), y(v.y), z(v.z)
		{}
		template<typename U, std::enable_if_t<HasXYZ<U>::value, int> = 0>
		JVector3& operator=(const U& v)
		{
			x = static_cast<ValueType>(v.x);
			y = static_cast<ValueType>(v.y);
			z = static_cast<ValueType>(v.z);
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
			return x != data.x && y != data.y && z != data.z;
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
		ValueType& operator[](const uint index)noexcept
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
		const ValueType& operator[](const uint index)const noexcept
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
		ValueType* Data()noexcept
		{
			return &x;
		}
	public:
		template<typename T1, typename T2>
		static float Dot(const JVector3<T1>& v1, const JVector3<T2>& v2) noexcept
		{
			return (float)(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
		}
		template<typename T1, typename T2>
		static JVector3 Cross(const JVector3<T1>& v1, const JVector3<T2>& v2) noexcept
		{
			return DirectX::XMVector3Cross(v1.ToXmV(), v2.ToXmV());
		}
		static JVector3 Min(const JVector3& a, const JVector3& b)noexcept
		{
			return JVector3(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);
		}
		static JVector3 Max(const JVector3& a, const JVector3& b)noexcept
		{
			return JVector3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
		}
	public:
		float Length()const noexcept
		{
			return abs(sqrt(x * x + y * y + z * z));
		}
		JVector3 Normalize()const noexcept
		{
			return DirectX::XMVector3Normalize(ToXmV());
		}
	public:
		static JVector3 Left()noexcept
		{
			return JVector3(-1, 0, 0);
		}
		static JVector3 Right()noexcept
		{
			return JVector3(1, 0, 0);
		}
		static JVector3 Up()noexcept
		{
			return JVector3(0, 1, 0);
		}
		static JVector3 Down()noexcept
		{
			return JVector3(0, -1, 0);
		}
		static JVector3 Front()noexcept
		{
			return JVector3(0, 0, 1);
		}
		static JVector3 Back()noexcept
		{
			return JVector3(0, 0, -1);
		}
		static JVector3 Zero() noexcept
		{
			return JVector3(0, 0, 0);
		}
		static JVector3 PositiveOne()noexcept
		{
			return JVector3(1, 1, 1);
		}
		static JVector3 NegativeOne()noexcept
		{
			return JVector3(-1, -1, -1);
		}
		static JVector3 PositiveInfV()noexcept
		{
			return JVector3(Base::PositiveInf<T>(), Base::PositiveInf<T>(), Base::PositiveInf<T>());
		}
		static JVector3 NegativeInfV()noexcept
		{
			return JVector3(Base::NegativeInf<T>(), Base::NegativeInf<T>(), Base::NegativeInf<T>());
		}
	public:
		template<typename T>
		auto ToSimilar()const noexcept -> std::conditional_t<HasXYZ<T>::value, T, void>
		{
			return T{ (float)x, (float)y, (float)z };
		}
		DirectX::XMVECTOR ToXmV(const float w = 1.0f)const noexcept
		{
			if constexpr (std::is_same_v<ValueType, T>)
				return DirectX::XMVectorSet((float)x, (float)y, (float)z, w);
			else
				return DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, w);
		}
		std::string ToString()const noexcept
		{
			return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 3;
		}
	public:
		void SetNanToZero()
		{
			if (std::isnan(x))
				x = (ValueType)0;
			if (std::isnan(y))
				y = (ValueType)0;
			if (std::isnan(z))
				z = (ValueType)0;
		}
	public:
		bool HasNan()const noexcept
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}
	};
	template<typename T>
	struct JVector4 : public JVectorBase
	{
	public:
		using ValueType = typename ValidVectorParameterDetermine_T<T>;
		using Base = JVectorBase;
	private:
		template<typename S, typename U = void>
		struct HasXYZW
		{
		public:
			static constexpr bool value = false;
		};
		template<typename S>
		struct HasXYZW<S, std::void_t<decltype(&S::x), decltype(&S::y), decltype(&S::z), decltype(&S::w)>>
		{
		public:
			static constexpr bool value = sizeof(S) == sizeof(JVector4<T>) &&
				std::is_convertible_v<decltype(S::x), ValueType> &&
				std::is_convertible_v<decltype(S::y), ValueType> &&
				std::is_convertible_v<decltype(S::z), ValueType> &&
				std::is_convertible_v<decltype(S::w), ValueType>;
		};
	public:
		static constexpr bool isValidValue = !std::is_same_v<ValueType, void>;
	public:
		union
		{
			struct
			{
				ValueType x;
				ValueType y;
				ValueType z;
				ValueType w;
			};
			struct
			{
				ValueType xyz[3];
				ValueType a;
			};
			ValueType value[4];
		};
	public:
		JVector4()
		{
			if constexpr (std::is_integral_v< ValueType> || std::is_floating_point_v<ValueType>)
				x = y = z = w = 0;
			else if constexpr (std::is_enum_v<ValueType>)
				x = y = z = w = (ValueType)0;
		}
		JVector4(const ValueType x, const ValueType y, const ValueType z, const ValueType w)
			: x(x), y(y), z(z), w(w)
		{}
		JVector4(const DirectX::XMFLOAT4& xm)
			: x(xm.x), y(xm.y), z(xm.z), w(xm.w)
		{}
#if defined(_XM_NO_INTRINSICS_) 
		JVector4(const __vector4& v)
			: x(v.vector4_f32[0]), y(v.vector4_f32[1])), z(v.vector4_f32[2])), w(v.vector4_f32[3]))
		{}
#elif defined(_XM_ARM_NEON_INTRINSICS_) 
		JVector4(const float32x4_t& v)
			: x(vgetq_lane_f32(v, 0)), y(vgetq_lane_f32(v, 1)), z(vgetq_lane_f32(v, 2)), w(vgetq_lane_f32(v, 3))
		{}
#elif defined(_XM_SSE_INTRINSICS_) 
		JVector4(const __m128& v)
		{
			_mm_store_ss(&x, v);
			_mm_store_ss(&y, XM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1)));
			_mm_store_ss(&z, XM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2)));
			_mm_store_ss(&w, XM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3)));
		}
#endif 
		JVector4(const float(&v)[4])
			:x(v[0]),
			y(v[1]),
			z(v[2]),
			w(v[3])
		{}
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
		JVector4(JVector4&&) = default;
		JVector4& operator=(JVector4&&) = default;
	public:
		template<typename U>
		JVector4(const JVector4<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
			w = static_cast<ValueType>(rhs.w);
		}
		template<typename U>
		JVector4& operator=(const JVector4<U>& rhs)
		{
			x = static_cast<ValueType>(rhs.x);
			y = static_cast<ValueType>(rhs.y);
			z = static_cast<ValueType>(rhs.z);
			w = static_cast<ValueType>(rhs.w);
			return *this;
		}
		template<typename U>
		JVector4(const JVector2<U>& v2, const typename JVector2<U>::ValueType z1, const typename JVector2<U>::ValueType w1)
			: x(v2.x), y(v2.y), z(z1), w(w1)
		{}
		template<typename U>
		JVector4(const JVector3<U>& v3, const typename JVector3<U>::ValueType w1)
			: x(v3.x), y(v3.y), z(v3.z), w(w1)
		{}
		template<typename U, std::enable_if_t<HasXYZW<U>::value, int> = 0>
		JVector4(const U& v)
			: x(v.x), y(v.y), z(v.z), w(v.w)
		{}
		template<typename U, std::enable_if_t<HasXYZW<U>::value, int> = 0>
		JVector4& operator=(const U& v)
		{
			x = static_cast<ValueType>(v.x);
			y = static_cast<ValueType>(v.y);
			z = static_cast<ValueType>(v.z);
			w = static_cast<ValueType>(v.w);
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
			return x != data.x && y != data.y && z != data.z && w != data.w;
		}
		ValueType& operator[](const uint index)noexcept
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
		const ValueType& operator[](const uint index)const noexcept
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
		ValueType* Data()noexcept
		{
			return &x;
		}
	public:
		template<typename T1, typename T2>
		static float Dot(const JVector4<T1>& v1, const JVector4<T2>& v2) noexcept
		{
			return (float)(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
		}
		template<typename T1, typename T2, typename T3>
		static JVector4 Cross(const JVector4<T1>& v1, const JVector4<T2>& v2, const JVector4<T3>& v3) noexcept
		{
			return DirectX::XMVector4Cross(v1.ToXmV(), v2.ToXmV(), v3.ToXmV());
		}
		static JVector4 Min(const JVector4& a, const JVector4& b)noexcept
		{
			return JVector4(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z, a.w < b.w ? a.w : b.w);
		}
		static JVector4 Max(const JVector4& a, const JVector4& b)noexcept
		{
			return JVector4(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z, a.w > b.w ? a.w : b.w);
		}
	public:
		float Length()const noexcept
		{
			return abs(sqrt(x * x + y * y + z * z + w * w));
		}
		JVector4 Normalize()const noexcept
		{
			return DirectX::XMVector4Normalize(ToXmV());
		}
	public:
		static JVector4 Zero() noexcept
		{
			return JVector4(0, 0, 0, 0);
		}
		static JVector4 One()noexcept
		{
			return JVector4(1, 1, 1, 1);
		}
		static JVector4 PositiveInfV()noexcept
		{
			return JVector4(Base::PositiveInf<T>(), Base::PositiveInf<T>(), Base::PositiveInf<T>(), Base::PositiveInf<T>());
		}
		static JVector4 NegativeInfV()noexcept
		{
			return JVector4(Base::NegativeInf<T>(), Base::NegativeInf<T>(), Base::NegativeInf<T>(), Base::NegativeInf<T>());
		}
	public:
		template<typename T>
		auto ToSimilar()const noexcept -> std::conditional_t<HasXYZW<T>::value, T, void>
		{
			return T{ (float)x, (float)y, (float)z, (float)w };
		}
		DirectX::XMVECTOR ToXmV()const noexcept
		{
			if constexpr (std::is_same_v<ValueType, T>)
				return DirectX::XMVectorSet((float)x, (float)y, (float)z, (float)w);
			else
				return DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		}
		std::string ToString()const noexcept
		{
			return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " " + std::to_string(w);
		}
	public:
		//For PrameterInfo
		static constexpr uint GetDigitCount()noexcept
		{
			return 4;
		}
	};

	using JVector2F = JVector2<float>;
	using JVector3F = JVector3<float>;
	using JVector4F = JVector4<float>;

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