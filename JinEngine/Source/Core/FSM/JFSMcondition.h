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
#include"JFSMinterface.h"   

namespace JinEngine
{
	namespace Core
	{
		class JFSMtransition;
		class JFSMparameter;
		class JFSMconditionPrivate;
		class JFSMcondition : public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMcondition)
		public: 
			class InitData : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				JUserPtr<JFSMtransition> ownerTransition;
			public:
				InitData(const JUserPtr<JFSMtransition>& ownerTransition);
				InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMtransition>& ownerTransition);
			public:
				InitData(const JTypeInfo& initTypeInfo, const JUserPtr<JFSMtransition>& ownerTransition);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name,  const size_t guid, const JUserPtr<JFSMtransition>& ownerTransition);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMconditionPrivate;
			class JFSMconditionImpl;
		private:
			std::unique_ptr<JFSMconditionImpl> impl;
		public: 
			JIdentifierPrivate& PrivateInterface()const noexcept override;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			JUserPtr<JFSMtransition> GetOwner()const noexcept;
			JUserPtr<JFSMparameter> GetParameter()const noexcept;
			float GetOnValue()const noexcept;
		public:
			void SetParameter(const JUserPtr<JFSMparameter>& newParam)noexcept;
			void SetOnValue(float newValue)noexcept;
		public:
			bool HasParameter()const noexcept;
			bool HasSameParameter(const size_t guid)const noexcept;
			bool IsSatisfied()const noexcept;
			bool PassDefectInspection()const noexcept;    
		protected:
			JFSMcondition(const InitData& initData);
			~JFSMcondition();
			JFSMcondition(JFSMcondition&& rhs) = default;
			JFSMcondition& operator=(JFSMcondition&& rhs) = default;
		};
 
	}
}