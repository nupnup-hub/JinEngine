#pragma once
#include<type_traits> 
#include"../Core/JDataType.h"  

namespace JinEngine
{
	namespace Core
	{
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
		struct StdArrayContainerDetermine : std::false_type {};

		template<typename T>
		struct StdArrayContainerDetermine<T, std::void_t<typename T::value_type>> : std::true_type
		{
		public:
			using ValueType = typename T::value_type;
		};
		template<typename T, typename = void, typename = void>
		struct StdMapDetermine : std::false_type {};

		template<typename T>
		struct StdMapDetermine<T, std::void_t<typename T::key_type>, std::void_t<typename T::value_type>> : std::true_type
		{
		public:
			using ValueType = typename T::value_type;
			using KeyType = typename T::key_type;
		};

		template<typename T, typename = void>
		struct JVectorDetermine : std::false_type
		{
		public:
			using ValueType = T;
		};

		template<typename T>
		struct JVectorDetermine<T, std::void_t<typename T::ValueType>> : std::true_type
		{
		public:
			using ValueType = typename T::ValueType;
		};

		template<typename T, typename = void>
		struct JTypeInfoDetermine : std::false_type {};

		template<typename T>
		struct JTypeInfoDetermine<T, std::void_t<typename T::ThisType>> : std::true_type {};
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

	}
}