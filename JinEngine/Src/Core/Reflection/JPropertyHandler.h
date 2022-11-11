#pragma once
#include"JReflectionMacro.h"
#include"JTypeInfoRegister.h"

namespace JinEngine
{
	namespace Core
	{
		class JPropertyHandlerBase
		{
			REGISTER_CLASS(JPropertyHandlerBase)
		public:
			virtual ~JPropertyHandlerBase() {}
		};

		template<typename T>
		class IPropertyHandler : public JPropertyHandlerBase
		{
			REGISTER_CLASS(IPropertyHandler)
		public:
			virtual T Get(void* object)const = 0;
			virtual void Set(void* object, const T& value) = 0;
		};

		template<typename Type, typename Field>
		class JStaticPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(JStaticPropertyHandler)
		private:
			using Pointer = Field*;
		private:
			Pointer ptr = nullptr;
		public:
			JStaticPropertyHandler(Pointer ptr)
				:ptr(ptr)
			{}
			Field Get([[maybe_unused]] void* object)const final
			{
				return *ptr;
			}
			void Set([[maybe_unused]] void* object, const Field& value)final
			{
				*ptr = value;
			}
		};

		template<typename Type, typename Field>
		class JMemberPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(JMemberPropertyHandler)
		private:
			using Pointer = Field Type::*;
		private:
			Pointer ptr = nullptr;
		public:
			JMemberPropertyHandler(Pointer ptr)
				:ptr(ptr)
			{ }
			Field Get(void* object)const final
			{ 
				return static_cast<Type*>(object)->*ptr;
			}
			void Set(void* object, const Field& value)final
			{
				static_cast<Type*>(object)->*ptr = value;
			}
		};

		template<typename Type, typename Field, typename GetPointer, typename SetPointer>
		class JStaticIndirectPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(JStaticIndirectPropertyHandler)
		private:
			GetPointer gPtr = nullptr;
			SetPointer sPtr = nullptr;
		public:
			JStaticIndirectPropertyHandler(GetPointer gPtr, SetPointer sPtr)
				:gPtr(gPtr), sPtr(sPtr)
			{}
			Field Get([[maybe_unused]] void* object)const final
			{
				return (*gPtr)();
			}
			void Set([[maybe_unused]] void* object, const Field& value)final
			{
				(*sPtr)(value);
			}
		};

		template<typename Type, typename Field, typename GetPointer, typename SetPointer>
		class JMemberIndirectPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(JMemberIndirectPropertyHandler)
		private:
			GetPointer gPtr = nullptr;
			SetPointer sPtr = nullptr;
		public:
			JMemberIndirectPropertyHandler(GetPointer gPtr, SetPointer sPtr)
				:gPtr(gPtr), sPtr(sPtr)
			{
			
			}
			Field Get(void* object)const final
			{ 
				return (static_cast<Type*>(object)->*gPtr)();
			}
			void Set(void* object, const Field& value)final
			{ 
				(static_cast<Type*>(object)->*sPtr)(value);
			}
		};
	}
}