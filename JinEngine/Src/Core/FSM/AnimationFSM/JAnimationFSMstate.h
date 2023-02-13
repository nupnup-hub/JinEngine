#pragma once
#include"JAnimationStateType.h" 
#include"../JFSMstate.h" 
#include"../../../Utility/JVector.h"
#include<unordered_map>

namespace JinEngine
{
	class JSkeletonAsset;  
	namespace Core
	{ 
		struct JAnimationShareData;
		struct JAnimationTime;
		class JAnimationFSMtransition;

		class JAnimationFSMstateStreamInteface
		{
		private:
			friend class JAnimationFSMdiagram;
		private:
			virtual J_FILE_IO_RESULT StoreData(std::wofstream& stream) = 0;
			virtual J_FILE_IO_RESULT LoadData(std::wifstream& stream) = 0;
		};

		class JAnimationFSMstate : public JFSMstate, 
			public JAnimationFSMstateStreamInteface
		{
			REGISTER_CLASS(JAnimationFSMstate) 
		private:  
			JVector2<float> pos;
		public:
			virtual J_ANIMATION_STATE_TYPE GetStateType()const noexcept = 0;
			virtual void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept = 0;
			virtual void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept = 0;
			virtual void Close(JAnimationShareData& animationShareData)noexcept = 0;
			virtual void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept = 0;
		public: 
			JVector2<float> GetPos()const noexcept;
			JAnimationFSMtransition* GetTransitionByIndex(uint index)noexcept;
			void SetPos(const JVector2<float>& newPos);
		public:   
			JAnimationFSMtransition* FindNextStateTransition(JAnimationTime& animationTime)noexcept;
		public:
			JAnimationFSMstateStreamInteface* StreamInterface();
		protected:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream) override; 
			J_FILE_IO_RESULT LoadData(std::wifstream& stream)override;
		protected:
			JAnimationFSMstate(const JFSMstateInitData& initData);
			~JAnimationFSMstate();
			JAnimationFSMstate(const JAnimationFSMstate& rhs) = delete;
			JAnimationFSMstate& operator=(const JAnimationFSMstate& rhs) = delete;
		};
	}
}