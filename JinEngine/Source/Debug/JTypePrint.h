#pragma once
#include<string>
#include<type_traits>
#include<iostream>
#include"../Utility/JTypeUtility.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		static T CastT(T t)
		{
			return static_cast<T>(t);
		}

		template<typename T>
		struct RemoveT
		{
		public:
			using result = std::remove_reference_t<T>;
		};

		//For Debug
		template<typename T>
		static void PrintType()
		{
			std::cout << "PrintType: " << std::endl;
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				std::cout << "const ";

			std::cout << typeid(T).name();

			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&";

			std::cout << std::endl;
		}
		template<typename T>
		static void PrintTypeAValue(T t)
		{
			std::cout << "PrintTypeAValue: " << std::endl;
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				std::cout << "const ";

			std::cout << typeid(T).name();
			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&";
			std::cout << std::endl;
			std::cout << t << std::endl;
		}
		template<typename T>
		static void PrintTypeUValue(T&& t)
		{
			std::cout << "PrintTypeUValue: " << std::endl;
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				std::cout << "const ";

			std::cout << typeid(T).name();

			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				std::cout << "&";

			std::cout << std::endl;
			std::cout << "decl" << std::endl;
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>>)
				std::cout << "const ";

			std::cout << typeid(decltype(t)).name();
			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>)
				std::cout << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>)
				std::cout << "&";

			std::cout << std::endl;

			if constexpr (std::is_array_v<std::remove_reference_t<decltype(t)>>)
			{
				using ValueType = RemoveAll_T<decltype(t)>;
				size_t eleSize = sizeof(t) / sizeof(ValueType);
				std::cout << "Type: " << typeid(ValueType).name() << std::endl;
				std::cout << "EleSize: " << eleSize << std::endl;
				std::cout << "Value: " << std::endl;
				for (int i = 0; i < eleSize; ++i)
					std::cout << t[i] << std::endl;
			}
			else  if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(t)>>)
			{
				using ValueType = RemoveAll_T<decltype(t)>;
				std::cout << "Type: " << typeid(ValueType).name() << std::endl;
				std::cout << "Value: " << std::endl;
				if constexpr (std::is_integral_v<ValueType>)
					std::cout << *t << std::endl;
			}
		}
		template<typename ...Param>
		static void PrintParams()
		{
			std::cout << std::endl;
			std::cout << "Print Param" << std::endl;
			(PrintType<Param>(), ...);
		}
		template<typename ...Param>
		static void PrintParamValues(Param&&... var)
		{
			std::cout << std::endl;
			std::cout << "Print Param values" << std::endl;
			(PrintTypeUValue(std::forward<Param>(var)), ...);
		}
		template<typename ...Param>
		static void PrintTupleParamType(std::tuple<Param...>& tuple)
		{
			std::cout << std::endl;
			std::cout << "Print Tuple Type Param" << std::endl;
			(PrintType<Param>(), ...);
		}
		template<size_t ...Is, typename Tulpe>
		static void PrintTupleParamValueType(std::index_sequence<Is...>, Tulpe& t)
		{
			std::cout << std::endl;
			std::cout << "Print Tuple Value Type Param" << std::endl;
			(PrintTypeAValue(std::get<Is>(std::move(t))), ...);
		}

		template<typename ...Param>
		class TestClass
		{
		public:
			TestClass() {}
			static void Print()
			{
				PrintParams< Param>();
			}
		};
	}
}