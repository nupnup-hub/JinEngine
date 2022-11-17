#pragma once
#include"../../../Utility/JMacroUtility.h" 
//#include"../../Reflection/JTypeInfo.h"
#include"../../Reflection/JTypeInfoRegister.h"
#include"../../Reflection/JReflectionMacro.h"
#include<windows.h>

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
			virtual Ret operator()(void* object, Param... var) = 0;
		};

		template<typename Ret, typename ...Param>
		class JStaticCallable final : public JCallableInterface<Ret, Param...>
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
			Ret operator()([[maybe_unused]] void* object, Param... var)
			{
				return (*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Ret, typename ...Param>
		class JStaticNCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JStaticNCallable)
		public:
			using Pointer = Ret(*)(Param...)noexcept;
		private:
			Pointer ptr;
		public:
			JStaticNCallable(Pointer ptr)
				:ptr(ptr)
			{ }
			Ret operator()([[maybe_unused]] void* object, Param... var)
			{
				return (*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Type, typename Ret, typename ...Param>
		class JMemberCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JMemberCallable)
		public: 
			using Pointer = Ret(Type::*)(Param...); 
		private:
			Pointer ptr;
		public:
			JMemberCallable(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(void* object, Param... var)
			{			 
				return (static_cast<Type*>(object)->*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Type, typename Ret, typename ...Param>
		class JMemberNCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JMemberNCallable)
		public:
			using Pointer = Ret(Type::*)(Param...)noexcept;
		private:
			Pointer ptr;
		public:
			JMemberNCallable(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(void* object, Param... var)
			{
				return (static_cast<Type*>(object)->*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Type, typename Ret, typename ...Param>
		class JMemberCCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JMemberCCallable)
		public:
			using Pointer = Ret(Type::*)(Param...)const;
		private:
			Pointer ptr;
		public:
			JMemberCCallable(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(void* object, Param... var)
			{
				return (static_cast<Type*>(object)->*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Type, typename Ret, typename ...Param>
		class JMemberCNCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS(JMemberCNCallable)
		public:
			using Pointer = Ret(Type::*)(Param...)const noexcept;
		private:
			Pointer ptr;
		public:
			JMemberCNCallable(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(void* object, Param... var)
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
		class JCallableHint final : public JCallableHintBase
		{
			REGISTER_CLASS(JCallableHint)
		public:
			JCallableHint() = default;
			~JCallableHint() = default;
		};


		template<typename Ret, typename ...Param>
		struct JStaticCallableType
		{
		public:
			using Ptr = Ret(*)(Param...);
			using Callable = JStaticCallable<Ret, Param...>;
		};
		template<typename Type, typename Ret, typename ...Param>
		struct JMemberCallableType
		{
		public:
			using Ptr = Ret(Type::*)(Param...);
			using Callable = JMemberCallable<Type, Ret, Param...>;
		};
	}
}

/*
MemberMethodInfoHandler(Ret(Type::*ptr)(Param...))
					:ptr(ptr)
*/