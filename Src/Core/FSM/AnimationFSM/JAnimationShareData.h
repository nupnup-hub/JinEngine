#pragma once 
#include<vector>
#include<unordered_map> 
#include"JAnimationStateType.h"
#include"../../JDataType.h"
#include"../../../Utility/JMathHelper.h"

namespace JinEngine
{
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
			static constexpr uint defaultLocalTransforCount = 2;
		};
		struct JAnimationShareData
		{
		public:
			//Share All diagram
			std::vector<DirectX::XMFLOAT4X4> localTransform[JAnimationFixedData::defaultLocalTransforCount];
			std::unordered_map<size_t, std::vector<JAnimationAdditionalBind>> additionalBind;

			//for Calculate
			std::vector<JAnimationIKCalculateJoint> ikJoint[JAnimationFixedData::defaultIKSlotCount];
			std::vector<JAnimationIKJointRate> ikRate;
			uint ikCount[JAnimationFixedData::defaultIKSlotCount];

			//Share a Diagram State
			J_ANIMATION_STATE_TYPE lastState[JAnimationFixedData::defaultLocalTransforCount];
			std::unordered_map<size_t, float> skeletonBlendRate[JAnimationFixedData::defaultLocalTransforCount];
		public:
			void Initialize()noexcept;
			void ClearSkeletonBlendRate(const uint index)noexcept;
			bool FindClipGuid(size_t& guid, const uint index)noexcept;
			//have to clear when start ik calculate
			void SetIKRate(const uint ikNumber)noexcept;
			void EnterCalculateIK()noexcept;
			void StuffIdentity(const uint localIndex)noexcept;
		};
	}
}