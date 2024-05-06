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
#include"JReflectionMacro.h"
#include"JTypeInfoRegister.h"
#include"JReflectionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class JPropertyHandlerBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JPropertyHandlerBase)
		public:
			virtual ~JPropertyHandlerBase() {}
		};

		template<typename T>
		class IPropertyHandler : public JPropertyHandlerBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(IPropertyHandler)
		public:
			virtual T Get(void* object)const = 0;
			virtual void Set(void* object, const T value) = 0;
		};

		template<typename Type, typename Field>
		class JStaticPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JStaticPropertyHandler)
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
			void Set([[maybe_unused]] void* object, const Field value)final
			{
				*ptr = value;
			}
		};

		template<typename Type, typename Field>
		class JMemberPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberPropertyHandler)
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
			void Set(void* object, const Field value)final
			{
				static_cast<Type*>(object)->*ptr = value;
			}
		};

		template<typename Type, typename Field, typename GetPointer, typename SetPointer>
		class JStaticIndirectPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JStaticIndirectPropertyHandler)
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
			void Set([[maybe_unused]] void* object, const Field value)final
			{
				(*sPtr)(value);
			}
		};

		template<typename Type, typename Field, typename GetPointer, typename SetPointer>
		class JMemberIndirectPropertyHandler final : public IPropertyHandler<Field>
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMemberIndirectPropertyHandler)
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
			void Set(void* object, const Field value)final
			{ 
				(static_cast<Type*>(object)->*sPtr)(value);
			}
		};
	}
}