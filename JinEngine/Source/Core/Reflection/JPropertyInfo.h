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
#include"JPropertyInfoInitializer.h" 
#include"JPropertyHandler.h" 
#include"JPropertyOptionInfo.h"
#include<assert.h> 

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JMethodInfo;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyInfoRegister;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyExInfoRegister;
		class JPropertyInfo final
		{ 
		private:
			friend class JTypeInfo;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyExInfoRegister;
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
				{
					assert(false && "JProperty Get Error"); 
				}
			}
			template<typename Field>
			Field UnsafeGet(void* object)
			{
				return static_cast<IPropertyHandler<Field>*>(handle)->Get(object);
			}
			template<typename Field>
			void Set(void* object, const Field value)
			{
				if (handle->GetTypeInfo().IsChildOf<IPropertyHandler<Field>>())
					static_cast<IPropertyHandler<Field>*>(handle)->Set(object, value);
				else
					assert(false && "JProperty Get Error");
			}
			//For enum property
			template<typename Field>
			void UnsafeSet(void* object, const Field value)
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