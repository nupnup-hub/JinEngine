#pragma once 
#include"JFSMcondition.h" 
#include"JFSMInterface.h"
#include"../JDataType.h"
#include<vector>  
#include<memory>
 
namespace JinEngine
{
	namespace Core
	{ 
		class JFSMstate;
		//class JFSMcondition;
		class JFSMparameter;
		class JFSMtransitionOwnerInterface;
		__interface JFSMparameterStorageUserAccess;

		class JFSMconditionOwnerInterface : public JTypeCashInterface<JFSMcondition>
		{
		private:
			friend class JFSMcondition;
		private:
			virtual JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept = 0; 
		};

		class JFSMtransition : public JFSMInterface, 
			public JFSMconditionOwnerInterface
		{
			REGISTER_CLASS(JFSMtransition)
		private:
			friend class JFSMstate;
		public:
			struct JFSMtransitionInitData : public JFSMIdentifierInitData
			{
			public:
				size_t outputGuid;
				JUserPtr<JFSMstate> owneState;		 
			public:
				JFSMtransitionInitData(const std::wstring& name,
					const size_t guid,
					const size_t outputGuid,
					JUserPtr<JFSMstate> owneState);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMtransitionInitData;
		public:
			static constexpr uint maxNumberOfCondition = 25;
			static constexpr float errorOnValue = -100000;
		private:
			JFSMtransitionOwnerInterface* ownerInterface; 
			const size_t outputStateGuid;
			std::vector<JFSMcondition*>conditionVec;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept;
			uint GetConditioCount()const noexcept;
			float GetConditionOnValue(const uint index)const noexcept;
			size_t GetOutputStateGuid()const noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)const noexcept;
			REGISTER_METHOD(GetConditionVec)
			std::vector<JFSMcondition*> GetConditionVec()const noexcept; 
		protected:
			//For editor gui
			REGISTER_METHOD(SetConditionVec)
			void SetConditionVec(std::vector<JFSMcondition*> vec)noexcept;
		private:
			JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept; 
		public: 
			bool IsSameDiagram(const size_t diagramGuid)const noexcept;
			bool HasSatisfiedCondition()const noexcept;
		private:
			bool AddType(JFSMcondition* newCondition)noexcept;
			bool RemoveType(JFSMcondition* condition)noexcept;		
		protected: 
			static JIdentifier* CreateCondition(JIdentifier* iden)noexcept;
			bool RemoveParameter(const size_t guid)noexcept;
		protected:
			virtual void Initialize()noexcept;
		private:
			void Clear()override;
		private:
			bool RegisterCashData()noexcept final;
			bool DeRegisterCashData()noexcept final;
		protected:
			JFSMtransition(const JFSMtransitionInitData& initData);
			~JFSMtransition();
			JFSMtransition(JFSMtransition&& rhs) = default;
			JFSMtransition& operator=(JFSMtransition&& rhs) = default;
		};
	}
}