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
#include"JFSMobjectType.h" 
#include"../Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMinterface : public JIdentifier
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMinterface)
		public:
			class InitData : public JIdentifier::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
			public:
				InitData(const JTypeInfo& initTypeInfo);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid);
			};
		protected: 
			class StoreData : public JDITypeDataBase
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
			public:
				JFSMinterface* fsmObj = nullptr;
			public:
				StoreData(JFSMinterface* fsmObj);
			public:
				bool IsValidData()const noexcept override;
				bool HasCorrectType(const JTypeInfo& correctType)const noexcept;
				bool HasCorrectChildType(const JTypeInfo& correctType)const noexcept;
			};
		public:
			virtual J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept = 0;
		protected:
			JFSMinterface(const InitData& initData);
		};
	}
}