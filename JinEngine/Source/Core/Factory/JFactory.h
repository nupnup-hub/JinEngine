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