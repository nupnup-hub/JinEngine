#pragma once
#include"../Utility/JTypeTraitUtility.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class JRestrictedRangeVar
		{
		private:
			using ValueType = std::conditional_t<IsNumber_V<T>, T, void>;
		private:
			ValueType value;
			ValueType minValue;
			ValueType maxValue;
		public:
			JRestrictedRangeVar(const T initValue, const T minValue, const T maxValue)
				:minValue(minValue), maxValue(maxValue)
			{
				Set(initValue);
			} 
		public:
			bool operator==(const JRestrictedRangeVar& rhs)const noexcept
			{
				return value == rhs.value &&
					minValue == rhs.minValue &&
					maxValue == rhs.maxValue;
			}
			bool operator!=(const JRestrictedRangeVar& rhs)const noexcept
			{
				return value != rhs.value ||
					minValue != rhs.minValue ||
					maxValue != rhs.maxValue;
			}
		public:
			ValueType Get()const noexcept
			{
				return value;
			}
			ValueType* GetPtr()
			{
				return &value;
			}
			ValueType& GetRef()
			{
				return value;
			}
			ValueType GetMin()const noexcept
			{
				return minValue;
			}
			ValueType GetMax()const noexcept
			{
				return maxValue;
			}
		public:
			void Set(const ValueType v)
			{
				value = std::clamp(v, minValue, maxValue);
			}
			void SetMin(ValueType v)
			{
				if (IsConstRange())
					return;

				if (v > maxValue)
					v = maxValue;
				minValue = v;
			}
			void SetMax(ValueType v)
			{
				if (IsConstRange())
					return;

				if (v < minValue)
					v = minValue;
				maxValue = v;
			}
		protected:
			virtual bool IsConstRange()const noexcept
			{
				return false;
			}
		public:
			std::string ToString()const noexcept
			{
				return std::to_string(value);
			}
		};

		template<typename T>
		class JRestrictedConstRangeVar : public JRestrictedRangeVar<T>
		{
		public:
			JRestrictedConstRangeVar(const T initValue, const T minValue, const T maxValue)
				:JRestrictedRangeVar<T>(initValue, minValue, maxValue)
			{}
		private:
			bool IsConstRange()const noexcept final
			{
				return true;
			}
		};

		template<typename T, T initValue, T minV, T maxV>
		class JRangeVarSetting
		{
		public:
			static constexpr T GetInit()
			{
				return initValue;
			}
			static constexpr T GetMin()
			{
				return minV;
			}
			static constexpr T GetMax()
			{
				return maxV;
			}
			template<typename U = T>
			static JRestrictedRangeVar<U> CreateRangeVar() noexcept
			{
				return JRestrictedRangeVar<U>(initValue, minV, maxV);
			}
			template<typename U = T>
			static JRestrictedConstRangeVar<U> CreateConstRangeVar() noexcept
			{
				return JRestrictedConstRangeVar<U>(initValue, minV, maxV);
			}
		};


	}
}