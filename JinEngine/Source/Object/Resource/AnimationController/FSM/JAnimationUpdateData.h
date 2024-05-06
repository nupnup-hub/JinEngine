/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once  
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