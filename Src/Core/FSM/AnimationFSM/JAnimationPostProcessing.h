#pragma once
#include<vector>
#include<DirectXMath.h>
#include"../../JDataType.h"
#include"../../../Object/Resource/Skeleton/Avatar/JAvatar.h"

namespace JinEngine
{
	class JAvatar;
	class JSkeletonAsset;
	namespace Core
	{
		struct JAnimationShareData;
		class JAnimationPostProcessing
		{
		private:
			static constexpr float defaulAngleRestriction = 45;
			static constexpr float minDistance = 0.1f;
			static constexpr int vectorSize = 20;
		public:
			static bool CalculateBindPoseIK(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				const J_AVATAR_JOINT start,
				const J_AVATAR_JOINT root,
				const uint ikNumber,
				const std::vector<DirectX::XMFLOAT3>& targetTranslation,
				const std::vector<DirectX::XMFLOAT4X4>& bindPose)noexcept;
		private:
			static void StuffBindPoseData(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeleton,
				DirectX::XMFLOAT3& effectorWorld,
				uint& ikCount,
				const uint ikNumber,
				const uint8 startJointRefIndex,
				const uint8 rootJointRefIndex,
				const std::vector<DirectX::XMFLOAT4X4>& bindPose)noexcept;
			static bool FindBindPoseIKValue(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeleton,
				const DirectX::XMFLOAT3& effectorWorld,
				const uint ikNumber,
				const uint ikCount,
				const uint loopCount)noexcept;
			static DirectX::XMVECTOR ConstrainAngle(const DirectX::XMFLOAT4& globalDeltaQ, const DirectX::XMFLOAT3& axis)noexcept;
		};
	}
}