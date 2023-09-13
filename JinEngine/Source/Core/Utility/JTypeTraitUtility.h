#pragma once
#include<type_traits> 
#include<vector>
#include<deque>
#include<map>
#include<unordered_map> 
#include"../JCoreEssential.h"  
#include"../Pointer/JOwnerPtr.h"
#include"../Math/JVector.h"
  
namespace JinEngine
{
	namespace Core
	{
		//type_traits을 활용한 도구모음 
#pragma region Struct
		template<class T, class U =
			typename std::remove_cv<
			typename std::remove_pointer<
			typename std::remove_reference<
			typename std::remove_extent<
			T
			>::type
			>::type
			>::type
			>::type
		> struct RemoveAll : RemoveAll<U> {};

		template<class T>
		struct RemoveAll<T, T>
		{
		public:
			using Type = T;
		};

		template<typename T>
		using RemoveAll_T = typename RemoveAll<T>::Type;

		template <bool>
		struct JSelect
		{
		public:
			template <typename T, typename>
			using Result = T;
		};

		template <>
		struct JSelect<false>
		{
		public:
			template <typename, typename T>
			using Result = T;
		};

		template<typename T>
		struct PointerCount
		{
		private:
			template<typename T, bool res>
			struct DoPointerCount
			{
			public:
				using Type = std::remove_pointer_t<T>;
				enum : uint8 { value = 1 + DoPointerCount <Type, std::is_pointer_v<Type>>::value };
			};
			template<typename T>
			struct DoPointerCount<T, false>
			{
			public:
				enum : uint8 { value = 0 };
			};
		public:
			enum : uint8 { value = DoPointerCount<std::remove_reference_t<T>, std::is_pointer_v<std::remove_reference_t<T>>>::value };
		};
		template<typename T>
		static constexpr uint8 PointerCount_V = PointerCount<T>::value;

		template<typename T>
		struct RemovePointer
		{
		private:
			template<typename T, bool res>
			struct DoRemovePointer
			{
			public:
				using Type = T;
			};
			template<typename T>
			struct DoRemovePointer<T, false>
			{
			public:
				using next = std::remove_pointer_t<T>;
				using Type = typename DoRemovePointer<next, !std::is_pointer_v<next>>::Type;
			};
		public:
			static constexpr bool hasLref = std::is_lvalue_reference_v<T>;
			static constexpr bool hasRref = std::is_rvalue_reference_v<T>;
			using OriType = std::remove_reference_t<T>;
			using RemovedPointer = typename DoRemovePointer<OriType, !std::is_pointer_v<OriType>>::Type;
			//using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			using AddReference = typename JSelect<hasLref>::template Result<std::add_lvalue_reference_t<RemovedPointer>, std::add_rvalue_reference_t<RemovedPointer>>;
			using Type = typename JSelect<hasRref || hasLref >::template Result<AddReference, RemovedPointer>;
		};
		template<typename T>
		using RemovePointer_T = typename RemovePointer<T>::Type;

		template<size_t count, typename T>
		struct AddPointer
		{
		public:
			using Type = typename AddPointer<count - 1, std::add_pointer_t<T>>::Type;
		};

		template<typename T>
		struct AddPointer<0, T>
		{
		public:
			using Type = T;
		};

		template<size_t count, typename T>
		using AddPointer_T = typename AddPointer<count, T>::Type;

		template<typename T>
		struct ArrayDimension
		{
		private:
			template<typename T, bool res>
			struct DoArrayDimension
			{
			public:
				using Type = std::remove_extent_t<T>;
				enum : uint8 { value = 1 + DoArrayDimension <Type, std::is_array_v<Type>>::value };
			};
			template<typename T>
			struct DoArrayDimension<T, false>
			{
			public:
				enum : uint8 { value = 0 };
			};
		public:
			enum : uint8 { value = DoArrayDimension<T, std::is_array_v<T>>::value };
		};
		template<typename T>
		static constexpr uint8 ArrayDimension_V = ArrayDimension<std::remove_reference_t<T>>::value;

		template<typename T>
		struct ArrayLastDimensionCount
		{
		private:
			template<bool res, typename T>
			struct Remove
			{
			public:
				enum : size_t { value = 0 };
			};
			template<bool res, typename T, size_t Ix>
			struct Remove<res, T[Ix]>
			{
			public:
				enum : size_t { value = Remove < ArrayDimension_V<T> >= 2, T>::value };
			};
			template<typename T, size_t Ix>
			struct Remove <false, T[Ix] >
			{
			public:
				enum : size_t { value = Ix };
			};
		public:
			enum : size_t { value = Remove<ArrayDimension_V<T> >= 2, T>::value };
		};

		template<typename T, bool res>
		struct TypeCondition;
		template<typename T>
		struct TypeCondition<T, true>
		{
		public:
			using Type = T;
		};
		template<typename T, bool res>
		using TypeCondition_T = typename TypeCondition<T, res>::Type;

		template<typename T, typename = void>
		struct StdStructureDetermine : std::false_type {using ValueType = T;};
		template<typename T>
		struct StdStructureDetermine<T, std::void_t<typename typename T::value_type>> : std::true_type { using ValueType = typename T::value_type; };

		template<typename T, typename = void>
		struct StdVectorDetermine : std::bool_constant<false>{ using ValueType = T; };

		template<typename T>
		struct StdVectorDetermine<T, std::void_t<typename T::value_type>>
			: std::bool_constant<std::is_same_v<std::vector<typename T::value_type>, T>>
		{
			using ValueType = typename T::value_type;
		};

		template<typename T, typename = void>
		struct StdDequeDetermine : std::bool_constant<false> { using ValueType = T; };

		template<typename T>
		struct StdDequeDetermine<T, std::void_t<typename T::value_type>>
			: std::bool_constant<std::is_same_v<std::deque<typename T::value_type>, T>>
		{
			using ValueType = typename T::value_type;
		};

		template<typename T, typename = void>
		struct StdArrayDetermine : std::bool_constant<false>{ using ValueType = T; };

		template<typename T>
		struct StdArrayDetermine<T, std::void_t<typename T::value_type>>
			: std::bool_constant<StdVectorDetermine<T>::value || StdDequeDetermine<T>::value>
		{ 
			using ValueType = typename T::value_type;
		};

		template<typename T, typename = void, typename = void>
		struct StdMapDetermine : std::bool_constant<false> {};

		template<typename T>
		struct StdMapDetermine<T, std::void_t<typename T::key_type>, std::void_t<typename T::value_type>>
			: std::bool_constant<std::is_same_v<std::map<typename T::key_type, typename T::value_type>, T>>
		{
		public:
			using ValueType = typename T::value_type;
			using KeyType = typename T::key_type;
		};

		template<typename T, typename = void, typename = void>
		struct StdUnorderedMapDetermine : std::bool_constant<false> {};

		template<typename T>
		struct StdUnorderedMapDetermine<T, std::void_t<typename T::key_type>, std::void_t<typename T::value_type>>
			: std::bool_constant<std::is_same_v<std::unordered_map<typename T::key_type, typename T::value_type>, T>>
		{
		public:
			using ValueType = typename T::value_type;
			using KeyType = typename T::key_type;
		};

		template<typename T, typename = void>
		struct JVectorDetermine : std::bool_constant<false>{ using ValueType = T; };

		template<typename T>
		struct JVectorDetermine<T, std::void_t<typename T::ValueType>> 
			: std::bool_constant <std::is_same_v<JVector2<typename T::ValueType>, T> || 
			std::is_same_v<JVector3<typename T::ValueType>, T> ||
			std::is_same_v<JVector4<typename T::ValueType>, T>>
		{
		public:
			using ValueType = typename T::ValueType;
		};

		template<typename T, typename = void>
		struct JTypeInfoDetermine : std::false_type {};

		template<typename T>
		struct JTypeInfoDetermine<T, std::void_t<typename T::ThisType>> : std::true_type {};
 
		template<typename T, typename = void>
		struct JUserPtrDetermine : std::bool_constant<false> { using ElementType = T; };

		//std::is_base_of_v<JPtrBase<JUserPtrDetermine<valueType>::ElementType>, T
		template<typename T>
		struct JUserPtrDetermine<T, std::void_t<typename T::ElementType>> 
			: std::bool_constant<std::is_same_v<JUserPtr<typename T::ElementType>, T>>
		{ 
			using ElementType = typename T::ElementType;
		};

		template<typename T>
		struct StdStructureLastDimValueType
		{
		private:
			template<typename T, bool isLastDim>
			struct LastDimDetermin
			{
			public:
				using ValueType = typename LastDimDetermin<typename StdStructureDetermine<T>::ValueType,
					StdStructureDetermine<T>::value>::ValueType;
			};
			template<typename T>
			struct LastDimDetermin<T, false>
			{
			public:
				using ValueType = T;
			};
		public:
			using ValueType = typename LastDimDetermin<typename StdStructureDetermine<T>::ValueType, 
				StdStructureDetermine<T>::value>::ValueType;
		};
 
		template<typename T, bool isUserPointer>
		struct JTypeVariableType;

		template<typename T>
		struct JTypeVariableType<T, false>{ public: using Type = T; };
		template<typename T>
		struct JTypeVariableType<T, true> { public: using Type = JUserPtr<T>; };

		template<typename T, bool isUserPointer, bool isVector>
		struct JStdVectorVariableType;
		template<typename T, bool isUserPointer>
		struct JStdVectorVariableType<T, isUserPointer, false> { public: using Type = typename JTypeVariableType<T, isUserPointer>::Type; };
		template<typename T, bool isUserPointer>
		struct JStdVectorVariableType<T, isUserPointer, true> { public: using Type = typename std::vector<typename JTypeVariableType<T, isUserPointer>::Type>; };
		 
		//caution!
		//simple struture for managed type of T [(uint)Enum::COUNT]
		//enum has to declare COUNT or Register
		//hasn't responsibility of destruction raw pointer
		template<typename T, typename Enum, bool canCopy = std::is_copy_constructible_v<T>>
		struct EnumCountArray
		{  
		public:
			T data[(uint)Enum::COUNT];
		public:
			T& operator[](const Enum index)const noexcept
			{
				return data[(uint)index];
			}
			T operator[](const Enum index)noexcept
			{ 
				return data[(uint)index];
			}
		public:
			void Clear()
			{
				for (uint i = 0; i < (uint)Enum::COUNT; ++i)
				{
					if constexpr (std::is_pointer_v<T>)
						data[i] = nullptr; 
				}
			}
		}; 
		template<typename T, typename Enum>
		struct EnumCountArray<T, Enum, false>
		{
		public:
			template<typename T, typename = void>
			struct HasReset : std::false_type{};
			template<typename T>
			struct HasReset<T, std::void_t<decltype(&T::reset)>> : std::true_type{};
		public:
			T data[(uint)Enum::COUNT];
		public:
			T& operator[](const Enum index)noexcept
			{
				return data[(uint)index];
			} 
		public:
			void Clear()
			{
				for (uint i = 0; i < (uint)Enum::COUNT; ++i)
				{
					if constexpr (std::is_pointer_v<T>)
						data[i] = nullptr;
					else if constexpr (HasReset<T>::value)
						data[i].reset();
				}
			}
		};


#pragma endregion

#pragma region Param
		template<typename T>
		static constexpr size_t ArrayLastDimensionCount_V = ArrayLastDimensionCount<std::remove_reference_t<T>>::value;
		template<typename T>
		static constexpr bool IsClass_V = std::is_class_v<RemoveAll_T<T>>;
		template<typename T>
		static constexpr bool IsEnum_V = std::is_enum_v<RemoveAll_T<T>>;
		template<typename T>
		static constexpr bool IsPointer_V = std::is_pointer_v<std::remove_reference_t<T>>;
		template<typename T>
		static constexpr bool IsArray_V = std::is_array_v<std::remove_reference_t<T>>;
		template<typename T>
		static constexpr bool IsConst_V = std::is_const_v<RemovePointer_T<std::remove_reference_t<T>>>;
		template<typename T>
		static constexpr bool IsLvalueRef_V = std::is_lvalue_reference_v<T>;
		template<typename T>
		static constexpr bool IsRvalueRef_V = std::is_rvalue_reference_v<T>;
		template<typename T>
		static constexpr bool IsRefernce_V = std::is_reference_v<T>;
#pragma endregion

#pragma region Method

		//add 2^n value enum 
		static int AddSQValue(const int ori, const int addValue)
		{
			return (ori | (addValue ^ (addValue & ori)));
		}
		template<typename enumType>
		auto AddSQValueEnum(const enumType ori, const enumType addValue) ->
			TypeCondition_T<enumType, std::is_enum_v< enumType>&& std::is_constructible_v<int, enumType>>
		{
			return (enumType)AddSQValue((int)ori, (int)addValue);
			//return (enumType)((int)ori | ((int)addValue ^ ((int)addValue & (int)ori)));
		}
		template<typename enumType, typename ...Param>
		auto AddSQValueEnum(enumType ori, Param... var) ->
			TypeCondition_T<enumType, std::is_enum_v< enumType>&& std::is_constructible_v<int, enumType>>
		{
			auto addSQValueEnumLam = [](enumType& ori, enumType addValue)
			{
				ori = AddSQValueEnum(ori, addValue);
			};
			((addSQValueEnumLam(ori, var)), ...);
			return ori;
		}

		static int MinusSQValue(const int ori, const int minusValue)
		{
			return (ori ^ (minusValue & ori));
		}
		//minus 2^n value enum
		template<typename enumType>
		auto MinusSQValueEnum(const enumType ori, const enumType minusValue) ->
			TypeCondition_T<enumType, std::is_enum_v< enumType>&& std::is_constructible_v<int, enumType>>
		{
			return (enumType)(MinusSQValue((int)ori, (int)minusValue));
		}
		//has 2^n value enum
		template<typename enumType, std::enable_if_t<std::is_enum_v<enumType>, int> = 0>
		bool HasSQValueEnum(const enumType ori, const enumType tar)
		{
			return (((int)ori & (int)tar) > 0);
		}

#pragma endregion

	}
}