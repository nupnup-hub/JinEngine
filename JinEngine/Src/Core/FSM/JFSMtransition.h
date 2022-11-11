#pragma once 
#include"JFSMcondition.h"
#include"../JDataType.h"
#include<vector> 
#include<unordered_map>
#include<memory>
 
namespace JinEngine
{
	namespace Core
	{ 
		class JFSMcondition; 
		class JFSMstate;
		class JFSMconditionWrap
		{
		private:
			JFSMcondition* condition;
			float onValue;
		public:
			JFSMconditionWrap(JFSMcondition* condition);
		public:
			JFSMcondition* GetCondition()noexcept;
			float GetOnValue()const noexcept;
			void SetCondition(JFSMcondition* newCondition)noexcept;
			void SetOnValue(float newValue)noexcept;
			bool IsSatisfied()const noexcept;
			bool PassDefectInspection()const noexcept;
		};
 
		class JFSMtransition
		{
		private:
			friend class JFSMstate;
			friend std::unique_ptr<JFSMtransition>::deleter_type;
		protected:
			using ConditionMap = std::unordered_map<size_t, JFSMcondition&>;
		public:
			static constexpr uint maxNumberOffCondition = 25;
			static constexpr float errorOnValue = -100000;
		private:
			std::vector<std::unique_ptr<JFSMconditionWrap>>conditionVec;
			const size_t outputStateGuid;
		public:
			uint GetConditioCount()const noexcept;
			float GetConditionOnValue(const uint index)const noexcept;
			size_t GetOutputStateGuid()const noexcept;
			bool HasSatisfiedCondition()const noexcept;
		protected: 
			JFSMcondition* GetConditionByIndex(const uint index)const noexcept; 
			void SetConditionOnValue(const uint index, const float onValue)noexcept;
			void SetCondition(const uint oldIndex, JFSMcondition* newCondition)noexcept;
		protected:
			JFSMconditionWrap* AddCondition(JFSMcondition* condition)noexcept;
			bool PopCondition(const size_t outputStateGuid)noexcept;
		protected:
			virtual void Initialize()noexcept;
		protected:
			JFSMtransition(const size_t outputStateGuid);
			~JFSMtransition();
			JFSMtransition(JFSMtransition&& rhs) = default;
			JFSMtransition& operator=(JFSMtransition&& rhs) = default;
		};
	}
}