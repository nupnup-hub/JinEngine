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