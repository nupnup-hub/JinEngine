#pragma once  
#include"JPropertyInfoInitializer.h" 
#include"JPropertyHandler.h" 
#include<assert.h>

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JPropertyInfo
		{
		private:
			friend class JTypeInfo;

		private:
			const std::string name;
			const JParameterHint fieldHint;
			PropertyHandlerBase* handle;
			JTypeInfo* ownerType;

		public:
			template<typename Field>
			JPropertyInfo(const JPropertyInfoInitializer<Field>& initializer)
				:name(initializer.name),
				fieldHint(initializer.fieldHint),
				handle(initializer.handlerBase),
				ownerType(initializer.ownerType)
			{}

			std::string Name()const noexcept;
			std::string FieldName()const noexcept; 
			const JParameterHint& GetHint()const noexcept;

			template<typename Field>
			Field& Get(void* object)
			{
				if (handle->GetTypeInfo().IsChildOf<IPropertyHandler<Field>>())
					return reinterpret_cast<IPropertyHandler<Field>*>(handle)->Get(object);
				else
					assert(false && "JProperty Get Error");
			}
			template<typename Field>
			Field& UnsafeGet(void* object)
			{
				return reinterpret_cast<IPropertyHandler<Field>*>(handle)->Get(object);
			}
			template<typename Field>
			void Set(void* object, const Field& value)
			{
				if (handle->GetTypeInfo().IsChildOf<IPropertyHandler<Field>>())
					reinterpret_cast<IPropertyHandler<Field>*>(handle)->Set(object, value);
				else
					assert(false && "JProperty Get Error");
			}
			template<typename Field>
			void UnsafeSet(void* object, const Field& value)
			{
				reinterpret_cast<IPropertyHandler<Field>*>(handle)->Set(object, value);
			}
		};
	}
}