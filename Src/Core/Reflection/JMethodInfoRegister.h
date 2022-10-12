#pragma once 
#include"JMethodInfo.h"   

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Pointer, Pointer ptr, typename Func>
		class JMethodInfoRegister;

		template<typename Type, typename Pointer, Pointer ptr, typename Ret, typename ...Param>
		class JMethodInfoRegister<Type, Pointer, ptr, Ret(Type::*)(Param...)>
		{
		public:
			JMethodInfoRegister(const std::string& name, const std::string& identificationName)
			{
				static JMemberCallable<Type, Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName, typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo()) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}
		};

		template<typename Type, typename Pointer, Pointer ptr, typename Ret, typename ...Param>
		class JMethodInfoRegister<Type, Pointer, ptr, Ret(*)(Param...)>
		{
		public:
			JMethodInfoRegister(const std::string& name, const std::string& identificationName)
			{
				static JStaticCallable<Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName,typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo()) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}
		};

		template<typename Type, typename Pointer, Pointer ptr>
		class JMethodInfoRegisterHelper
		{
		public:
			JMethodInfoRegisterHelper(const std::string& name, const std::string& identificationName)
			{
				static JMethodInfoRegister<Type, Pointer, ptr, Pointer> paramType{ name, identificationName };
			}
		};
	}
}