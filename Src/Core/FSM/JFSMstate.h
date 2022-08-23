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
			const size_t guid;
			std::vector<std::unique_ptr<JFSMtransition>> transition;
			bool decidedNextState = false;
		public:
			JFSMstate(const std::string& name, const size_t guid);
			virtual ~JFSMstate();
			JFSMstate(const JFSMstate& rhs) = delete;
			JFSMstate& operator=(const JFSMstate& rhs) = delete;
		public:
			std::string GetName()noexcept;
			size_t GetGuid()noexcept;
		protected:
			void SetName(const std::string& name);
			void SetTransitionCondtion(const size_t outputStateGuId, const uint conditionIndex, JFSMcondition* newCondition)noexcept;
			void SetTransitionCondtionOnValue(const size_t outputStateGuId, const uint conditionIndex, const float onValue)noexcept;

			virtual void Initialize()noexcept;
			virtual JFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept;
			virtual bool RemoveTransition(const size_t outputStateGuId)noexcept;

			void EnterState()noexcept;
			bool AddTransitionCondition(const size_t outputStateGuId, JFSMcondition* condition)noexcept;
			bool RemoveCondition(const size_t guid)noexcept;
			bool RemoveTransitionCondition(const size_t outputStateGuId, const size_t conditionGuid)noexcept;
			virtual void Clear()noexcept;
		};
	}
}