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
#include"../../Skeleton/Avatar/JAvatarJointType.h"
#include"../../../../Core/Math/JMatrix.h"

namespace JinEngine
{
	class JSkeletonAsset;

	struct JAnimationAdditionalBind;
	class JAnimationUpdateData;
	class JAnimationPostProcessing;
	class JAnimationRetargeting
	{
	public:
		static bool CalculateAdditionalBindPose(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind)noexcept;
	private:
		static void StuffTargetTranslation(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JVector3<float>>& bindTargetLocalT,
			std::vector<JVector3<float>>& bindTargetWorldT)noexcept;
		static void ReBuildTargetTranslation(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JVector3<float>>& bindTargetLocalT,
			std::vector<JVector3<float>>& bindTargetWorldT,
			uint8 index)noexcept;
		static void GetBindPoseAdditionalTransform(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const std::vector<JVector3<float>>& tarModTranslation,
			std::vector<JAnimationAdditionalBind>& additionalBind)noexcept;
		static void StuffBindPoseIKResult(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<JMatrix4x4>& modBindPose)noexcept;
		static void AdjustJoint(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JMatrix4x4>& modBindPose)noexcept;
		static void RotateJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const uint8 srcIndex,
			const uint8 tarIndex,
			const uint8 srcChildIndex,
			const uint8 tarChildIndex,
			std::vector<JMatrix4x4>& modBindPose)noexcept;
		static void TransformJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const uint8 srcIndex,
			const uint8 tarIndex,
			const uint8 srcChildIndex,
			const uint8 tarChildIndex,
			std::vector<JMatrix4x4>& modBindPose)noexcept;

		static void UpdateParentBindPose(JSkeletonAsset* skeletonAsset,
			std::vector<JMatrix4x4>& bindPose,
			const DirectX::XMMATRIX preParentBindPose,
			const uint8 index);
		static void DebugQGapPrint00(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<JMatrix4x4>& modBindPose)noexcept;
		static void DebugQGapPrint01(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<JMatrix4x4>& modBindPose)noexcept;
	};
}