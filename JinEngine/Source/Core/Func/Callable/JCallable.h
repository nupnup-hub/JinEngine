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
#include"../../Utility/JMacroUtility.h" 
//#include"../../Reflection/JTypeInfo.h"
#include"../../Reflection/JTypeInfoRegister.h"
#include"../../Reflection/JReflectionInfo.h"
#include"../../Reflection/JReflectionMacro.h" 

namespace JinEngine
{
	namespace Core
	{ 
		class JCallableBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCallableBase)
		public:
			virtual ~JCallableBase() {}
		};

		template<typename Ret, typename ...Param>
		class JCallableInterface : public JCallableBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCallableInterface) 
		public:
			virtual Ret operator()(void* object, Param... var) = 0;
		};

		template<typename Ret, typename ...Param>
		class JStaticCallable final : public JCallableInterface<Ret, Param...>
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JStaticCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JStaticNCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberNCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberCCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberCNCallable)
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
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCallableHintBase)
		public:
			virtual ~JCallableHintBase() {}
		};

		template<typename Ret, typename ...DecayParam>
		class JCallableHint final : public JCallableHintBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCallableHint)
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