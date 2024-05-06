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
#include<DirectXMath.h> 
#include"../../Skeleton/Avatar/JAvatar.h"
#include"../../../../Core/Math/JMatrix.h"

namespace JinEngine
{
	class JAvatar;
	class JSkeletonAsset;
	class JAnimationUpdateData;
	class JAnimationPostProcessing
	{
	private:
		static constexpr float defaulAngleRestriction = 45;
		static constexpr float minDistance = 0.1f;
		static constexpr int vectorSize = 20;
	public:
		static bool CalculateBindPoseIK(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const J_AVATAR_JOINT start,
			const J_AVATAR_JOINT root,
			const uint ikNumber,
			const std::vector<JVector3<float>>& targetTranslation,
			const std::vector<JMatrix4x4>& bindPose)noexcept;
	private:
		static void StuffBindPoseData(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeleton,
			JVector3<float>& effectorWorld,
			uint& ikCount,
			const uint ikNumber,
			const uint8 startJointRefIndex,
			const uint8 rootJointRefIndex,
			const std::vector<JMatrix4x4>& bindPose)noexcept;
		static bool FindBindPoseIKValue(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeleton,
			const JVector3<float>& effectorWorld,
			const uint ikNumber,
			const uint ikCount,
			const uint loopCount)noexcept;
		static JVector4<float> ConstrainAngle(const JVector4<float>& globalDeltaQ, const JVector3<float>& axis)noexcept;
	};
}