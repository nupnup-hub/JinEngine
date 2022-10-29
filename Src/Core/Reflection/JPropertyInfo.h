#pragma once  
#include"JPropertyInfoInitializer.h" 
#include"JPropertyHandler.h" 
#include"JPropertyOptionInfo.h"
#include<assert.h>
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JMethodInfo;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyInfoRegister;

		class JPropertyInfo
		{
		private:
			friend class JTypeInfo;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
		private:
			const std::string name;
			const JParameterHint fieldHint;
			JPropertyHandlerBase* handle;
			JTypeInfo* ownerType;
		private:
			JPropertyOptionInfo* optionInfo;
		public:
			std::string Name()const noexcept;
			std::string FieldName()const noexcept; 
			JParameterHint GetHint()const noexcept;
			JTypeInfo* GetTypeInfo()const noexcept;
			JPropertyOptionInfo* GetOptionInfo()const noexcept;
		public:
			template<typename Field>
			Field Get(void* object)
			{
				if (handle->GetTypeInfo().IsChildOf<IPropertyHandler<Field>>())
					return static_cast<IPropertyHandler<Field>*>(handle)->Get(object);
				else
					assert(false && "JProperty Get Error");
			}
			template<typename Field>
			Field UnsafeGet(void* object)
			{
				return static_cast<IPropertyHandler<Field>*>(handle)->Get(object);
			}
			template<typename Field>
			void Set(void* object, const Field& value)
			{
				if (handle->GetTypeInfo().IsChildOf<IPropertyHandler<Field>>())
					static_cast<IPropertyHandler<Field>*>(handle)->Set(object, value);
				else
					assert(false && "JProperty Get Error");
			}
			template<typename Field>
			void UnsafeSet(void* object, const Field& value)
			{
				static_cast<IPropertyHandler<Field>*>(handle)->Set(object, value);
			}
		private:
			template<typename Field>
			JPropertyInfo(const JPropertyInfoInitializer<Field>& initializer)
				:name(initializer.name),
				fieldHint(initializer.fieldHint),
				handle(initializer.handlerBase),
				ownerType(initializer.ownerType),
				optionInfo(initializer.optionInfo)
			{}
			~JPropertyInfo() = default;
		};
	}
}