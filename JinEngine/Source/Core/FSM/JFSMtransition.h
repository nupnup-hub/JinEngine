#pragma once  
#include"JFSMinterface.h"
#include"../JCoreEssential.h"
#include<vector>  
#include<memory>
 
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