#pragma once 
#include<vector>
#include<unordered_map> 
#include"JAnimationStateType.h"
#include"JAnimationTime.h"
#include"JBlender.h" 
#include"../../JDataType.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		struct JAnimationAdditionalBind
		{
		public:
			DirectX::XMFLOAT4X4 transform = JMathHelper::Identity4x4();
			bool isIKModified = false;
		};
		struct JAnimationIKCalculateJoint
		{
		public:
			DirectX::XMFLOAT4X4 movedTransform;
			DirectX::XMFLOAT4X4 initTransform;
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
		class JGameTimer; 
		class JAnimationUpdateData
		{
		public:
			struct DiagramData
			{
			public:
				JAnimationFSMstate* nowState = nullptr;
				JAnimationFSMstate* nextState = nullptr;
				JAnimationFSMtransition* nowTransition = nullptr;
				JAnimationFSMtransition* preTransition = nullptr;
				JBlender blender;
				float weight;
			public:
				JAnimationTime animationTimes[JAnimationFixedData::defaultCrossFadingCount];
				DirectX::XMFLOAT4X4 worldTransform[JAnimationFixedData::defaultCrossFadingCount][JSkeletonFixedData::maxJointCount];
			public:
				void Clear();
			};
		public:
			JGameTimer* timer = nullptr;
			JSkeletonAsset* modelSkeleton = nullptr;
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
			std::unordered_map<std::wstring, float> paramValueMap; 
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
			void RegisterParameter(const std::wstring& str, const float value)noexcept;
			float GetParameterValue(const std::wstring& str)const noexcept;
			void SetParameterValue(const std::wstring& str, const float value)noexcept;
		};
	}
}