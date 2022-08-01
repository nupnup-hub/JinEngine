#pragma once  
#include"../Exception/JExceptionMacro.h"
#include"../Singleton/JSingletonHolder.h"
#include"../Func/Callable/JCallable.h"
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		//class JCallableBase;
		//template<typename T, typename ...Param>class JCallableInterface;
		template<typename IdentifierType, bool isThrowException, typename Ret, typename ...Param>
		class JFactory
		{
		private:
			std::unordered_map<IdentifierType, JCallableBase*> funcMap;
		public:
			bool Regist(IdentifierType idenType, JCallableInterface<Ret, Param...>* callable)
			{
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(funcMap.find(idenType) != funcMap.end());
				}
				else
				{
					ReturnIfFailedN(funcMap.find(idenType) != funcMap.end(), false);
				}
				funcMap.emplace(idenType, callable);
				return true;
			}

			Ret Invoke(IdentifierType idenType, Param&&... value)
			{
				auto func = funcMap.find(idenType);
				if constexpr (isThrowException)
				{
					ThrowIfFailedN(func != funcMap.end());
				}
				else
				{
					ReturnIfFailedN(func != funcMap.end(), nullptr);
				}
				return (*static_cast<JCallableInterface<Ret, Param...>*>(func->second))(nullptr, std::forward<Param>(value)...);
			}
		};
	}
}