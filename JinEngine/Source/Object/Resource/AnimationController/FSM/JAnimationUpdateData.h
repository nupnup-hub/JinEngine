#pragma once 
#include<vector>
#include<unordered_map> 
#include"JAnimationStateType.h"
#include"JAnimationTime.h"
#include"JBlender.h" 
#include"../../Skeleton/JSkeletonFixedData.h"
#include"../../../../Core/Pointer/JOwnerPtr.h" 
#include"../../../../Core/Math/JMathHelper.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		class JGameTimer;
	}
	struct JAnimationAdditionalBind
	{
	public:
		JMatrix4x4 transform = JMatrix4x4::Identity();
		bool isIKModified = false;
	};
	struct JAnimationIKCalculateJoint
	{
	public:
		JMatrix4x4 movedTransform;
		JMatrix4x4 initTransform;
		uint8 jointIndex;
		uint8 jointRefIndex;
	};
	struct JAnimationIKJointRate
	{
	public:
		float rate;
		uint count;
	};
	struct JAnimationFixedData
	{
	public:
		static constexpr uint ikVectorSize = 20;
		static constexpr uint defualtBindPoseIkLoopCount = 75;
		static constexpr uint defaultIKSlotCount = 4;
		static constexpr uint defaultCrossFadingCount = 2;
		static constexpr uint fsmDiagramMaxCount = 8;
	};

	class JAnimationFSMstate;
	class JAnimationFSMtransition;
	class JAnimationUpdateData
	{
	public:
		struct DiagramData
		{
		public:
			Core::JUserPtr<JAnimationFSMstate> nowState = nullptr;
			Core::JUserPtr<JAnimationFSMstate> nextState = nullptr;
			Core::JUserPtr<JAnimationFSMtransition> nowTransition = nullptr;
			Core::JUserPtr<JAnimationFSMtransition>  preTransition = nullptr;
			JBlender blender;
			float weight;
		public:
			JAnimationTime animationTimes[JAnimationFixedData::defaultCrossFadingCount];
			JMatrix4x4 worldTransform[JAnimationFixedData::defaultCrossFadingCount][JSkeletonFixedData::maxJointCount];
		public:
			void Clear();
		};
	public:
		Core::JGameTimer* timer = nullptr;
		Core::JUserPtr<JSkeletonAsset> modelSkeleton = nullptr;
	public:
		DiagramData diagramData[JAnimationFixedData::fsmDiagramMaxCount];
	public:
		//Share All diagram		
		std::unordered_map<size_t, std::vector<JAnimationAdditionalBind>> additionalBind;

		//for Calculate
		std::vector<JAnimationIKCalculateJoint> ikJoint[JAnimationFixedData::defaultIKSlotCount];
		std::vector<JAnimationIKJointRate> ikRate;
		uint ikCount[JAnimationFixedData::defaultIKSlotCount];

		//Share a Diagram State
		J_ANIMATION_STATE_TYPE lastState[JAnimationFixedData::defaultCrossFadingCount];
		std::unordered_map<size_t, float> skeletonBlendRate[JAnimationFixedData::defaultCrossFadingCount];
	public:
		std::unordered_map<size_t, float> paramValueMap;
	public:
		void Initialize()noexcept;
		void ClearSkeletonBlendRate(const uint index)noexcept;
		bool FindClipGuid(size_t& guid, const uint index)noexcept;
		//have to clear when start ik calculate
	public:
		void SetIKRate(const uint ikNumber)noexcept;
		void EnterCalculateIK()noexcept;
	public:
		void StuffIdentity(const uint layerNumber, const uint updateNumber)noexcept;
	public:
		void RegisterParameter(const size_t guid, const float value)noexcept;
		float GetParameterValue(const size_t guid)const noexcept;
		void SetParameterValue(const size_t guid, const float value)noexcept;
	};
}