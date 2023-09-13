#pragma once 
#include"JMethodInfoInitializer.h" 
#include"JMethodOptionInfo.h"
#include"../Func/Callable/JCallable.h"   
#include"../Utility/JTypeTraitUtility.h" 
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JCallableBase;
		class JTypeInfo;
		template<typename Type, typename Pointer, Pointer ptr> class JMethodInfoRegister;

		class JMethodInfo final
		{
		private:
			friend class JTypeInfo;
			template<typename Type, typename Pointer, Pointer ptr> friend class JMethodInfoRegister;
		private:
			const std::string name;
			const std::string identificationName;
			JParameterHint returnHint;
			std::vector<JParameterHint> parameterHint;
			JCallableBase* callHandle; 
			JCallableHintBase* hintHandle;
			JTypeInfo* ownerType; 
		private:
			JMethodOptionInfo* optionInfo;
		public:
			std::string Name()const noexcept;
			std::string ReturnFieldName()const noexcept; 
			std::string ParameterFieldName()const noexcept;
			uint ParameterCount()const noexcept;
			JParameterHint GetReturnHint()const noexcept;
			JTypeInfo* GetTypeInfo()const noexcept;
			JMethodOptionInfo* GetOptionInfo()const noexcept;
		public:
			template<typename T>
			bool IsConvertibleRet()
			{
				return returnHint.IsConvertible<T>();
			}
			template<typename T>
			bool IsConvertibleParam(const size_t index)
			{  
				//(IsConvertibleRet<Param>() && ...);
				return parameterHint[index].IsConvertible<T>();
			}
			template<typename ...Param>
			bool IsConvertibleParam()
			{
				if (sizeof...(Param) != parameterHint.size())
					return false;

				return IsConvertible<Param...>(parameterHint, std::make_index_sequence<sizeof...(Param)>());
			}
		public:
			template<typename Ret, typename ...Param>
			Ret Invoke(void* object, Param&&... var)
			{
				if (callHandle->GetTypeInfo().IsChildOf<JCallableInterface<Ret, Param...>>())
					return (*static_cast<JCallableInterface<Ret, Param...>*>(callHandle))(object, std::forward<Param>(var)...);
				else if (hintHandle->GetTypeInfo().IsChildOf<JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...>>())
				{ 
					using indexSeq = std::make_index_sequence<sizeof...(Param)>; 
					if (IsConvertible<Param...>(parameterHint, indexSeq()))
						return (*static_cast<JCallableInterface<Ret, Param...>*>(callHandle))(object, std::forward<decltype(var)> (var)...);
					else
						assert(false && "JMethodInfo Invoke Error");
				}
				else
					assert(false && "JMethodInfo Invoke Error");
			}
			template<typename Ret, typename ...Param>
			Ret UnsafeInvoke(void* object, Param&&... var)
			{
				return (*static_cast<JCallableInterface<Ret, Param...>*>(callHandle))(object, std::forward<Param>(var)...);
			}
		private:
			template<typename Ret, size_t ...Is, typename ...Param>
			Ret InvokeEx(void* object, std::index_sequence<Is...>, Param&&... var)
			{
				return (*static_cast<JCallableInterface<Ret, Param...>*>(callHandle))(object, Forward(parameterHint[Is], std::forward<Param>(var))...);
			}
		private:
			template<typename Ret, typename ...Param>
			JMethodInfo(const JMethodInfoInitializer<Ret, Param...>& initializer)
				:name(initializer.name),
				identificationName(initializer.identificationName),
				returnHint(initializer.returnHint),
				parameterHint(initializer.parameterHint),
				callHandle(initializer.callHandle),
				hintHandle(initializer.hintHandle),
				ownerType(initializer.ownerType),
				optionInfo(initializer.optionInfo)
			{}
			~JMethodInfo() = default;
		};
	}
}

/*
				else if (decayHandle->GetTypeInfo().IsChildOf<DecayCallable<Ret, JTypeConvertible<Param>::DecayType...>>())
				{
					//callHandle이 상속한 JCallableInterface로 static_cast
					//safety하지 않지만 callHandle은 항상 JCallableInterface<ret, ...Param>형식을 상속하며
					//JCallableInterface<ret, ...Param>은 Interface class이므로 유연성을 위해 사용하기로 한다
					auto iHandle = static_cast<JCallableInterface<Ret, JTypeConvertible<Param>::DecayType...>*>(callHandle);
					if (iHandle->IsConvertible<JTypeConvertible<Param>::DecayType...>())
					{
						if constexpr (std::is_void_v<Ret>)
							(*iHandle)(object, JTypeConvertible<Param>::ToDecay(var)...);
						else
							return (*iHandle)(object, JTypeConvertible<Param>::ToDecay(var)...);
					}
					else
					{
						assert(false && "JMethodInfo Invoke Error");
						return NULL;
					}
				}
*/