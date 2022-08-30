#pragma once
#include"JAnimationStateType.h" 
#include"../JFSMstate.h" 
#include<unordered_map>

namespace JinEngine
{
	class JSkeletonAsset;  
	namespace Core
	{
		struct JAnimationShareData;
		struct JAnimationTime;
		struct JFSMLoadGuidMap;
		__interface IJFSMconditionStorageUser;
		class JAnimationFSMtransition;
		class JAnimationFSMstate : public JFSMstate
		{ 
		protected:
			using StateMap = std::unordered_map<size_t, JAnimationFSMstate&>;
			using ConditionMap = std::unordered_map<size_t, JFSMcondition&>;
		private: 
			std::vector<JAnimationFSMtransition*> transitionCash;
		public:
			JAnimationFSMstate(const std::wstring& name, const size_t guid);
			~JAnimationFSMstate();
			virtual J_ANIMATION_STATE_TYPE GetStateType()const noexcept = 0;
			virtual void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept = 0;
			virtual void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept = 0;
			virtual void Close(JAnimationShareData& animationShareData)noexcept = 0;
			virtual void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept = 0;
			 
			std::vector<JAnimationFSMtransition*>& GetTransitionVector()noexcept;

			JAnimationFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept;
			bool RemoveTransition(const size_t outputStateGuid)noexcept;
			JAnimationFSMtransition* FindNextStateTransition(JAnimationTime& animationTime)noexcept;
		public:
			J_FILE_IO_RESULT StoreIdentifierData(std::wofstream& stream);
			virtual J_FILE_IO_RESULT StoreContentsData(std::wofstream& stream); 
			virtual J_FILE_IO_RESULT LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap, IJFSMconditionStorageUser& iConditionUser);
		};
	}
}