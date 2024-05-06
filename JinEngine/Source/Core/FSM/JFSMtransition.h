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
#include"../JCoreEssential.h" 
 
namespace JinEngine
{
	namespace Core
	{ 
		class JFSMstate;
		class JFSMcondition; 

		class JFSMtransitionPrivate;
		class JFSMtransition : public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMtransition)
		public: 
			class InitData : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				JUserPtr<JFSMstate> inState;
				JUserPtr<JFSMstate> outState;
			public:
				InitData(const JUserPtr<JFSMstate>& inState, const JUserPtr<JFSMstate>& outState);
				InitData(const std::wstring& name,
					const size_t guid,
					const JUserPtr<JFSMstate>& inState,
					const JUserPtr<JFSMstate>& outState);
			public:
				InitData(const JTypeInfo& initTypeInfo, 
					const JUserPtr<JFSMstate>& inState,
					const JUserPtr<JFSMstate>& outState);
				InitData(const JTypeInfo& initTypeInfo, 
					const std::wstring& name,
					const size_t guid,
					const JUserPtr<JFSMstate>& inState,
					const JUserPtr<JFSMstate>& outState);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMtransitionPrivate;
			class JFSMtransitionImpl;
		private:
			std::unique_ptr<JFSMtransitionImpl> impl;
		public:
			static constexpr uint GetMaxConditionCapacity()noexcept
			{
				return 50;
			}
			JIdentifierPrivate& PrivateInterface()const noexcept override;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept;
			JUserPtr<JFSMstate> GetInState()const noexcept;
			JUserPtr<JFSMstate> GetOutState()const noexcept;
			uint GetConditioCount()const noexcept;
			float GetConditionOnValue(const uint index)const noexcept;
			size_t GetOutputStateGuid()const noexcept;
			JUserPtr<JFSMcondition> GetConditionByIndex(const uint index)const noexcept;
			std::vector<JUserPtr<JFSMcondition>> GetConditionVec()const noexcept;
		protected:
			void SetConditionVec(std::vector<JUserPtr<JFSMcondition>> vec)noexcept;
		public:  
			bool HasSatisfiedCondition()const noexcept; 
		protected: 
			static JUserPtr<JIdentifier> CreateCondition(const JUserPtr<JIdentifier>& iden)noexcept;
		protected:
			void Initialize()noexcept;
		protected:
			JFSMtransition(const InitData& initData);
			~JFSMtransition();
			JFSMtransition(JFSMtransition&& rhs) = default;
			JFSMtransition& operator=(JFSMtransition&& rhs) = default;
		};
	}
}