#pragma once
#include<string>
#include<vector>
#include<memory>
#include"../JDataType.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class JFSMtransition;

		class JFSMstate
		{
			friend class JFSMdiagram;
		public:
			static constexpr uint maxNumberOffTransistions = 100;
		private:
			std::string name;
			size_t id;
			std::vector<std::unique_ptr<JFSMtransition>> transition;
			bool decidedNextState = false;
		public:
			JFSMstate() = default;
			virtual ~JFSMstate();
			JFSMstate(const JFSMstate& rhs) = delete;
			JFSMstate& operator=(const JFSMstate& rhs) = delete;

			std::string GetName()noexcept;
			size_t GetId()noexcept;
		protected:
			void SetName(const std::string& name, const size_t id);
			void SetTransitionCondtion(const size_t outputId, JFSMcondition* newCondition, const uint oldConditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const size_t outputId, const uint conditionIndex, const float onValue)noexcept;

			virtual void Initialize()noexcept;
			virtual JFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept;
			virtual bool EraseTransition(const size_t outputId)noexcept;

			void EnterState()noexcept;
			bool AddTransitionCondition(const size_t outputId, JFSMcondition* condition)noexcept;
			bool EraseCondition(const std::string& conditionName)noexcept;
			bool EraseTransitionCondition(const size_t outputId, const std::string& conditionName)noexcept;
			virtual void Clear()noexcept;
		};
	}
}