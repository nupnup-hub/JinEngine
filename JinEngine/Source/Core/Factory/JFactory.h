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
#include"../Exception/JExceptionMacro.h"
#include"../Singleton/JSingletonHolder.h"
#include"../Func/Callable/JCallable.h"  
#include<windows.h>
namespace JinEngine
{
	namespace Core
	{
		//class JCallableBase;
		//template<typename T, typename ...Param>class JCallableInterface;
		template<typename IdentifierType, bool isThrowException, bool useCallable, typename Ret, typename ...Param>
		class JFactory
		{
		private:
			std::unordered_map<IdentifierType, JCallableBase*> funcMap;
		public:
			bool Register(IdentifierType idenType, JCallableInterface<Ret, Param...>* callable)
			{ 
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(funcMap.find(idenType) == funcMap.end());
				}
				else if (funcMap.find(idenType) != funcMap.end())
					return false; 
				funcMap.emplace(idenType, callable);
				return true;
			}
			Ret Invoke(IdentifierType idenType, Param... value)
			{ 
				auto func = funcMap.find(idenType);
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(func != funcMap.end());
				}
				else if (funcMap.find(idenType) == funcMap.end())
					return nullptr;
				 
				return (*static_cast<JCallableInterface<Ret, Param...>*>(func->second))(nullptr, std::forward<Param>(value)...);
			}
		protected:
			void Clear()
			{
				funcMap.clear();
			}
		};
		template<typename IdentifierType, bool isThrowException, typename Ret, typename ...Param>
		class JFactory<IdentifierType, isThrowException, false, Ret, Param...>
		{
		public:
			using CreatePtr = Ret(*)(Param...);
		private:
			std::unordered_map<IdentifierType, CreatePtr> funcMap;
		public:
			bool Register(IdentifierType idenType, CreatePtr ptr)
			{
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(funcMap.find(idenType) == funcMap.end());
				}
				else if (funcMap.find(idenType) != funcMap.end())
					return false;
				funcMap.emplace(idenType, ptr);
				return true;
			}
			Ret Invoke(IdentifierType idenType, Param... value)
			{
				auto func = funcMap.find(idenType);
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(func != funcMap.end());
				}
				else if (funcMap.find(idenType) == funcMap.end())
					return nullptr;

				return (func->second)(nullptr, std::forward<Param>(value)...);
			}
		protected:
			void Clear()
			{
				funcMap.clear();
			}
		};
	}
}