#pragma once 
#include"JMethodInfo.h"    
//#include"JTypeInfoOption.h"
//#include"JGuiWidgetInfoHandle.h"

namespace JinEngine
{
	namespace Core
	{
		class JGuiWidgetInfo;
		template<typename Type> class JMethodGuiWidgetInfoHandle;
		template<typename Type> class JMethodReadOnlyGuiWidgetInfoHandle;
		//중복된 클래스 생성을 막기위해 Pointer & ptr template에 추가
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
			/*JMethodInfoRegisterHelper(const std::string& name, const std::string& identificationName, Widget&&... var)
			{ 
				//std::forward<Widget>(var)...
				static JMethodOptionInfo methodOptionInfo{};
				static JMethodInfoRegister<Type, Pointer, ptr> methodInfo{ name, identificationName, ptr, methodOptionInfo, std::forward<Widget>(var)... };
			}*/
			JMethodInfoRegisterHelper(const std::string& name, const std::string& identificationName)
			{
				static JMethodOptionInfo methodOptionInfo{};
				static JMethodInfoRegister<Type, Pointer, ptr> methodInfo{ name, identificationName, ptr, methodOptionInfo };
			}
		};
		 

		template<typename Type, typename GetPointer, GetPointer getPtr>
		class JMethodReadOnlyGuiWidgetRegister
		{
		public:
			template<typename ...Widget>
			JMethodReadOnlyGuiWidgetRegister(const std::string& displayName, const std::string& getName, Widget&&... var)
			{
				PassingAddReadOnlyGuiWidgetHandle(getPtr, displayName, getName, std::forward<Widget>(var)...);
			}
		private:
			//Read only
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(Type::* ptr)(),
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(Type::* ptr)()noexcept,
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(Type::* ptr)()const,
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(Type::* ptr)()const noexcept,
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(*ptr)(),
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddReadOnlyGuiWidgetHandle(Ret(*ptr)()noexcept,
				const std::string& displayName,
				const std::string& getName,
				Widget&&... var)
			{
				AddReadOnlyGuiWidgetInfoHandle<Ret>(displayName, getName, std::forward<Widget>(var)...);
			}
		private:
		private:
			template<typename Ret, typename ...Widget>
			void AddReadOnlyGuiWidgetInfoHandle(const std::string& displayName, const std::string& getName, Widget&&... var)
			{
				JMethodInfo* getInfo = Type::StaticTypeInfo().GetMethod(getName);
				const bool isValid = getInfo->IsConvertibleRet<Ret>();
				if (isValid && getInfo != nullptr)
				{
					Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle
					(std::make_unique<JMethodReadOnlyGuiWidgetInfoHandle<Ret>>(getInfo, displayName, std::forward<Widget>(var)...));
				}
			}
		};

		template<typename Type, typename GetPointer, GetPointer getPtr>
		class JMethodGuiWidgetRegister
		{
		public:			
			template<typename ...Widget>
			JMethodGuiWidgetRegister(const std::string& displayName, const std::string& getName, const std::string& setName, Widget&&... var)
			{
				PassingAddGuiWidgetHandle(getPtr, displayName, getName, setName, std::forward<Widget>(var)...);
			}
		private:
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(Type::* ptr)(),
				const std::string& displayName,
				const std::string& getName, 
				const std::string& setName,
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(Type::* ptr)()const, 
				const std::string& displayName,
				const std::string& getName, 
				const std::string& setName, 
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(Type::* ptr)()noexcept,
				const std::string& displayName,
				const std::string& getName, 
				const std::string& setName, 
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(Type::* ptr)()const noexcept,
				const std::string& displayName,
				const std::string& getName,
				const std::string& setName, 
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(*ptr)(),
				const std::string& displayName,
				const std::string& getName,
				const std::string& setName, 
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
			template<typename Ret, typename ...Widget>
			void PassingAddGuiWidgetHandle(Ret(*ptr)()noexcept,
				const std::string& displayName,
				const std::string& getName, 
				const std::string& setName, 
				Widget&&... var)
			{
				AddGuiWidgetInfoHandle<Ret>(displayName, getName, setName, std::forward<Widget>(var)...);
			}
		private:
			template<typename Ret, typename ...Widget>
			void AddGuiWidgetInfoHandle(const std::string& displayName, const std::string& getName, const std::string& setName, Widget&&... var)
			{
				JMethodInfo* getInfo = Type::StaticTypeInfo().GetMethod(getName);
				JMethodInfo* setInfo = Type::StaticTypeInfo().GetMethod(setName);

				const bool isValid = getInfo->IsConvertibleRet<Ret>() && setInfo->IsConvertibleParam<Ret>();
				if (isValid && getInfo != nullptr && setInfo != nullptr)
				{
					Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle
					(std::make_unique<JMethodGuiWidgetInfoHandle<Ret>>(getInfo, setInfo, displayName, std::forward<Widget>(var)...));
				}
			}
		};
	}
}