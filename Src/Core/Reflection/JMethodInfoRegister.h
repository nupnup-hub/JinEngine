#pragma once 
#include"JMethodInfo.h"   

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Pointer, Pointer ptr>
		class JMethodInfoRegister
		{
		public:
			template<typename Ret, typename ...Param>
			JMethodInfoRegister(const std::string& name,
				const std::string& identificationName,
				Ret(Type::*ptr)(Param...),
				JMethodOptionInfo& methodOptionInfo)
			{
				static JMemberCallable<Type, Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName, typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo(), methodOptionInfo) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}
			template<typename Ret, typename ...Param>
			JMethodInfoRegister(const std::string& name,
				const std::string& identificationName,
				Ret(Type::* ptr)(Param...)const,
				JMethodOptionInfo& methodOptionInfo)
			{
				static JMemberCCallable<Type, Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName, typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo(), methodOptionInfo) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}
			template<typename Ret, typename ...Param>
			JMethodInfoRegister(const std::string& name,
				const std::string& identificationName,
				Ret(Type::* ptr)(Param...)noexcept,
				JMethodOptionInfo& methodOptionInfo)
			{
				static JMemberNCallable<Type, Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName, typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo(), methodOptionInfo) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}
			template<typename Ret, typename ...Param>
			JMethodInfoRegister(const std::string& name,
				const std::string& identificationName,
				Ret(Type::* ptr)(Param...)const noexcept,
				JMethodOptionInfo& methodOptionInfo)
			{
				static JMemberCNCallable<Type, Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName, typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo(), methodOptionInfo) };
				static bool init = false;
				if (!init)
				{
					Type::StaticTypeInfo().AddMethodInfo(&jMethodInfo);
					init = true;
				}
			}

			template<typename Ret, typename ...Param>
			JMethodInfoRegister(const std::string& name,
				const std::string& identificationName,
				Ret(*ptr)(Param...),
				JMethodOptionInfo& methodOptionInfo)
			{
				static JStaticCallable<Ret, Param...> callHandle{ ptr };
				static JCallableHint<RemoveAll_T<Ret>, RemoveAll_T<Param>...> hintHandle;
				static JMethodInfo jMethodInfo{ JMethodInfoInitializer<Ret, Param...>(name, identificationName,typeid(Pointer).name(), callHandle, hintHandle, Type::StaticTypeInfo(), methodOptionInfo) };
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
			JMethodInfoRegisterHelper(const std::string& name, const std::string& identificationName, std::unique_ptr<JGuiWidgetInfo>&& widgetInfo)
			{ 
				static JMethodOptionInfo methodOptionInfo{ std::move(widgetInfo) };
				static JMethodInfoRegister<Type, Pointer, ptr> methodInfo{ name, identificationName, ptr, methodOptionInfo };
			}
			JMethodInfoRegisterHelper(const std::string& name, const std::string& identificationName)
			{
				static JMethodOptionInfo methodOptionInfo{};
				static JMethodInfoRegister<Type, Pointer, ptr> methodInfo{ name, identificationName, ptr, methodOptionInfo };
			}
		};
	}
}