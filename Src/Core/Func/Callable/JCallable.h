#pragma once
#include"../../../Utility/JMacroUtility.h" 
#include"../../Reflection/JTypeInfo.h"  
#include"../../Reflection/JReflectionMacro.h"

namespace JinEngine
{
	namespace Core
	{
		class JCallableBase
		{
			REGISTER_CLASS(JCallableBase)
		public:
			virtual ~JCallableBase() {}
		};

		template<typename Ret, typename ...Param>
		class JCallableInterface : public JCallableBase
		{
			REGISTER_CLASS(JCallableInterface) 
		public:
			virtual Ret operator()(void* object, Param&&... var) = 0;
		};

		template<typename Ret, typename ...Param>
		class JStaticCallable : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JStaticCallable)
		public:
			using Pointer = Ret(*)(Param...); 
		private:
			Pointer ptr;
		public:
			JStaticCallable(Pointer ptr)
				:ptr(ptr)
			{ }
			Ret operator()([[maybe_unused]] void* object, Param&&... var)
			{
				return (*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Type, typename Ret, typename ...Param>
		class JMemeberCallable : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JMemeberCallable)
		public: 
			using Pointer = Ret(Type::*)(Param...); 
		private:
			Pointer ptr;
		public:
			JMemeberCallable(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(void* object, Param&&... var)
			{			 
				return (static_cast<Type*>(object)->*ptr)(std::forward<Param>(var)...);
			}
		};

		class JCallableHintBase
		{
			REGISTER_CLASS(JCallableHintBase)
		public:
			virtual ~JCallableHintBase() {}
		};

		template<typename Ret, typename ...DecayParam>
		class JCallableHint : public JCallableHintBase
		{
			REGISTER_CLASS(JCallableHint)
		public:
			JCallableHint() = default;
			~JCallableHint() = default;
		};
	}
}

/*
MemberMethodInfoHandler(Ret(Type::*ptr)(Param...))
					:ptr(ptr)
*/