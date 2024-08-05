/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"JTypeTraitUtility.h"  
#include"../Empty/JEmptyType.h" 

namespace JinEngine
{
	namespace Core
	{
		/**
		* for basically data type like int, float, chat, enum... 
		*/
		template<typename T, T ...Type>
		struct JDefinedTypeSequence
		{
		public:
			using ValueType = T;
		public:
			static constexpr uint count = sizeof...(Type);
		public: 
			static int Index(T t)
			{  
				int index = 0;
				int result = invalidIndex;

				((t == Type ? (result = index): ++index), ...);
				return result;
			}
			static T At(int i)
			{
				int index = 0;
				T result = (T)invalidIndex;
				 
				auto lam = [](T& result, T type, int& index, int i)
				{
					if (i == index)
						result = type;
					else
						++index;
				};

				(lam(result, Type, index, i), ...);
				//((i == index ? (result = Type) : ++index), ...);
				return result;
			}
		public:
			template<size_t ...Is>
			static void StuffTypeSequenceOrder(std::index_sequence<Is...>, int* order, const int maxCount)
			{
				for (int i = 0; i < maxCount; ++i)
					order[i] = invalidIndex;

				uint orderNumber = 0;
				for (uint i = 0; i < sizeof...(Is); ++i)
				{
					order[(int)At(i)] = orderNumber;
					++orderNumber;
				} 
			}
		}; 

		//template<typename T, typename U, T _T, U _U>
		//struct JStaticTuple
		//위에 타입은 상수값에 따라서 각각 다른 타입이 만들어지므로 JDefinedTypeSequence를 이용할 수 없다.
		//복수의 타입의 결합이 필요하므로 위와같은 경우는 아래와 같이 Param으로 하나의 타입으로 취급하여 사용한다
		//2024-07-29

		template<typename ...Param>
		struct JStaticTupleSequence
		{
		public:
			using Typelist = typename Core::MakeTypelist<Param...>::Result;
		private:
			template<typename T, typename = void>
			struct FirstTypeDefine
			{
				using Type = void;
			};
			template<typename T>
			struct FirstTypeDefine<T, std::void_t<typename T::Head::FirstType>>
			{
				using Type = typename T::Head::FirstType;
			};
			template<typename T, typename = void>
			struct SecondTypeDefine
			{
				using Type = void;
			};
			template<typename T>
			struct SecondTypeDefine<T, std::void_t<typename T::Head::SecondType>>
			{
				using Type = typename T::Head::SecondType;
			};
		public: 
			using FirstType = typename FirstTypeDefine<Typelist>::Type;
			using SecondType = typename SecondTypeDefine<Typelist>::Type;
			static_assert(Core::Length<Typelist>::value != 0, "Can't use 0 sequence");
			static_assert(!std::is_void_v<FirstType> && !std::is_void_v<SecondType>, "Invalid sequence type");
		public:
			static constexpr uint count = sizeof...(Param);
		public:
			template<typename T>
			static int Index(T t)
			{
				int index = 0;
				int result = invalidIndex;
	 
				if constexpr (std::is_same_v<T, FirstType>)
					((t == Param::first ? (result = index) : ++index), ...);
				else if constexpr (std::is_same_v<T, SecondType>)
					((t == Param::second ? (result = index) : ++index), ...);
				return result;
			} 
			template<typename T>
			static T At(int i)
			{
				int index = 0;
				T result = (T)invalidIndex;

				auto lam = [](T& result, T type, int& index, int i)
				{
					if (i == index)
						result = type;
					else
						++index;
				};

				if constexpr (std::is_same_v<T, FirstType>)
				{
					(lam(result, Param::first, index, i), ...);
					//((i == index ? (result = Param::first) : ++index), ...);
				}
				else if constexpr (std::is_same_v<T, SecondType>)
				{
					(lam(result, Param::second, index, i), ...);
					//((i == index ? (result = Param::second) : ++index), ...);
				}
				return result;
			} 
		public:
			template<typename T, size_t ...Is>
			static void StuffTypeSequenceOrder(std::index_sequence<Is...>, int* order, const int maxCount)
			{
				for (int i = 0; i < maxCount; ++i)
					order[i] = invalidIndex;

				uint orderNumber = 0;
				for (uint i = 0; i < sizeof...(Is); ++i)
				{
					order[(int)At<T>(i)] = orderNumber;
					++orderNumber;
				}
			}
		};
	}
}