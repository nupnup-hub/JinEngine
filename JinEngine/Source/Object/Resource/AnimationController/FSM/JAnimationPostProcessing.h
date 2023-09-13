#pragma once
#include<vector>
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