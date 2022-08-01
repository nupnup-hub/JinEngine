#pragma once
#include"JReflectionMacro.h"
#include"JTypeInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class PropertyHandlerBase
		{
			REGISTER_CLASS(PropertyHandlerBase)
		public:
			virtual ~PropertyHandlerBase() {}
		};

		template<typename T>
		class IPropertyHandler : public PropertyHandlerBase
		{
			REGISTER_CLASS(IPropertyHandler)
		public:
			virtual T& Get(void* object)const = 0;
			virtual void Set(void* object, const T& value)const = 0;
		};

		template<typename Type, typename Field>
		class StaticPropertyHandler : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(StaticPropertyHandler)
				using Pointer = Field*;
		private:
			Pointer ptr;
		public:
			StaticPropertyHandler(Pointer ptr)
				:ptr(ptr)
			{}
			Field& Get([[maybe_unused]] void* object)const override
			{
				return *ptr;
			}
			void Set([[maybe_unused]] void* object, const Field& value)const override
			{
				*ptr = value;
			}
		};

		template<typename Type, typename Field>
		class MemberPropertyHandler : public IPropertyHandler<Field>
		{
			REGISTER_CLASS(MemberPropertyHandler)
				using Pointer = Field Type::*;
		private:
			Pointer ptr;
		public:
			MemberPropertyHandler(Pointer ptr)
				:ptr(ptr)
			{ }
			Field& Get(void* object)const override
			{
				return static_cast<Type*>(object)->*ptr;
			}
			void Set(void* object, const Field& value)const override
			{
				static_cast<Type*>(object)->*ptr = value;
			}
		};
	}
}