#pragma once 
#include<vector> 
#include<memory>
#include"JFSMcondition.h"
#include"../JDataType.h"
 
namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		struct FSMConditionWrap
		{
		public:
			JFSMcondition* condition;
			float onValue;
		public:
			FSMConditionWrap(JFSMcondition* condition);
			bool IsSatisfied()const noexcept;
			bool PassDefectInspection()const noexcept;
		};
		class JFSMtransition
		{
			friend class JFSMstate;
		public:
			static constexpr uint maxNumberOffCondition = 25;
		private:
			std::vector<std::unique_ptr<FSMConditionWrap>>conditionVec;
			const size_t outputId;
		public:
			JFSMtransition(const size_t outputId);
			virtual ~JFSMtransition() = default;
			size_t GetOutputId()noexcept;
			bool HasSatisfiedCondition()noexcept;
		protected:
			virtual void Initialize()noexcept;
			void SetConditionOnValue(const uint conditionIndex, const float onValue)noexcept;
			void SetCondition(const uint oldConditionIndex, JFSMcondition* newCondition)noexcept;
			bool AddCondition(JFSMcondition* condition)noexcept;
			bool PopCondition(const std::string& name)noexcept;
		};
	}
}