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
#include"../JCoreEssential.h"
#include"../Reflection/JTypeBase.h" 
#include"../DI/JDIDataBase.h" 

namespace JinEngine
{
	namespace Core
	{   
		class JIdentifierPrivate;
		class JIdentifier : public JTypeBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JIdentifier)
		public:
			struct InitData : public JDITypeDataBase	// it is same as metaData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				const JTypeInfo& initTypeInfo;
			public:
				std::wstring name;
				size_t guid;
			public:
				InitData(const JTypeInfo& initTypeInfo);
				InitData(const JTypeInfo& initTypeInfo, const size_t guid);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid);
			public:
				const JTypeInfo& InitDataTypeInfo()const noexcept;
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JIdentifierPrivate;
			class JIdentifierImpl;
		private:
			std::unique_ptr<JIdentifierImpl> impl;
		public:
			static JIdentifierPrivate* PrivateInterface(const size_t typeGuid)noexcept;
			virtual JIdentifierPrivate& PrivateInterface()const noexcept = 0;
		public:
			std::wstring GetName() const noexcept;
			std::wstring GetNameWithType()const noexcept; 
			static std::wstring GetDefaultName(const JTypeInfo& info)noexcept; 
		public:
			void SetIdentifiable()noexcept;
			void SetUnIdentifiable()noexcept;
			virtual void SetName(const std::wstring& newName)noexcept;
		public:
			virtual bool CanControlIdentifiable()const noexcept;
		public:  
			static bool BeginCopy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to);
			static bool BeginDestroy(Core::JIdentifier* ptr);
		protected:
			static bool BeginForcedDestroy(Core::JIdentifier* ptr);
			static void RegisterPrivateInterface(const JTypeInfo& info, JIdentifierPrivate& p);	//can't register abstract class 
		public:
			template<typename T, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, T>, int> = 0>
			static std::wstring GetDefaultName()noexcept
			{
				return GetDefaultName(T::StaticTypeInfo());
			}
		public:
			template<typename T, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, T>, int> = 0>
			static void AlignByName(std::vector<JUserPtr<T>>& vec, const bool isAscending = true)noexcept
			{
				auto aSortLam = [](const JUserPtr<JIdentifier>& a, const JUserPtr<JIdentifier>& b)
				{
					return tolower(a->GetName()[0]) < tolower(b->GetName()[0]);
				};
				auto dSortLam = [](const JUserPtr<JIdentifier>& a, const JUserPtr<JIdentifier>& b)
				{
					return tolower(a->GetName()[0]) > tolower(b->GetName()[0]);
				};
				if (isAscending)
					std::sort(vec.begin(), vec.end(), aSortLam);
				else
					std::sort(vec.begin(), vec.end(), dSortLam);
			}
		protected:
			JIdentifier(const InitData& initData);
			~JIdentifier();
		};
	} 
}