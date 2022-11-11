#pragma once
#include<vector>
#include<DirectXMath.h>
#include"../../JDataType.h"
#include"../../../Object/Resource/Skeleton/Avatar/JAvatarJointType.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		struct JAnimationAdditionalBind;
		struct JAnimationShareData;
		class JAnimationPostProcessing;
		class JAnimationRetargeting
		{
		public:
			static bool CalculateAdditionalBindPose(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<JAnimationAdditionalBind>& additionalBind)noexcept;
		private:
			static void StuffTargetTranslation(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<DirectX::XMFLOAT3>& bindTargetLocalT,
				std::vector<DirectX::XMFLOAT3>& bindTargetWorldT)noexcept;
			static void ReBuildTargetTranslation(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<DirectX::XMFLOAT3>& bindTargetLocalT,
				std::vector<DirectX::XMFLOAT3>& bindTargetWorldT,
				uint8 index)noexcept;
			static void GetBindPoseAdditionalTransform(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				const std::vector<DirectX::XMFLOAT3>& tarModTranslation,
				std::vector<JAnimationAdditionalBind>& additionalBind)noexcept;
			static void StuffBindPoseIKResult(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<JAnimationAdditionalBind>& additionalBind,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;
			static void AdjustJoint(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;
			static void RotateJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				const uint8 srcIndex,
				const uint8 tarIndex,
				const uint8 srcChildIndex,
				const uint8 tarChildIndex,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;
			static void TransformJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				const uint8 srcIndex,
				const uint8 tarIndex,
				const uint8 srcChildIndex,
				const uint8 tarChildIndex,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;

			static void UpdateParentBindPose(JSkeletonAsset* skeletonAsset,
				std::vector<DirectX::XMFLOAT4X4>& bindPose,
				const DirectX::XMMATRIX preParentBindPose,
				const uint8 index);
			static void DebugQGapPrint00(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<JAnimationAdditionalBind>& additionalBind,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;
			static void DebugQGapPrint01(JAnimationShareData& animationShareData,
				JSkeletonAsset* srcSkeletonAsset,
				JSkeletonAsset* tarSkeletonAsset,
				std::vector<JAnimationAdditionalBind>& additionalBind,
				std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept;
		};
	}
}