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


#include"JAnimationPostProcessing.h"
#include"JAnimationUpdateData.h"
#include"../../Skeleton/JSkeleton.h"
#include"../../Skeleton/JSkeletonAsset.h" 
#include"../../Skeleton/JSkeletonFixedData.h" 
#include"../../Skeleton/Avatar/JAvatar.h" 
#include"../../../../Core/Math/JMathHelper.h"
#include<algorithm>

namespace JinEngine
{
	using namespace DirectX;
	bool JAnimationPostProcessing::CalculateBindPoseIK(JAnimationUpdateData* updateData,
		JSkeletonAsset* srcSkeletonAsset,
		JSkeletonAsset* tarSkeletonAsset,
		const J_AVATAR_JOINT start,
		const J_AVATAR_JOINT root,
		const uint ikNumber,
		const std::vector<JVector3<float>>& targetTranslation,
		const std::vector<JMatrix4x4>& bindPose)noexcept
	{
		JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
		JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();

		uint8 srcStRefIndex;
		uint8 tarStRefIndex;
		srcAvatar->FindCommonReferenceIndexEndToRoot(start, root, tarAvatar, srcStRefIndex, tarStRefIndex);

		if (tarStRefIndex == JSkeletonFixedData::incorrectJointIndex || srcStRefIndex == JSkeletonFixedData::incorrectJointIndex)
			return false;

		const uint8 srcRootRefIndex = srcAvatar->jointReferenceMap.find(root)->second;
		const uint8 tarRootRefIndex = tarAvatar->jointReferenceMap.find(root)->second;

		const uint8 srcStIndex = srcAvatar->jointReference[srcStRefIndex];
		const uint8 tarStIndex = tarAvatar->jointReference[tarStRefIndex];

		JVector3<float> endEffectorf = targetTranslation[tarStIndex];
		StuffBindPoseData(updateData, srcSkeletonAsset, endEffectorf, updateData->ikCount[ikNumber], ikNumber, srcStRefIndex, srcRootRefIndex, bindPose);
		updateData->SetIKRate(ikNumber);
		bool res = FindBindPoseIKValue(updateData, srcSkeletonAsset, endEffectorf, ikNumber, updateData->ikCount[ikNumber], JAnimationFixedData::defualtBindPoseIkLoopCount);

		return res;
	}
	void JAnimationPostProcessing::StuffBindPoseData(JAnimationUpdateData* updateData,
		JSkeletonAsset* srcSkeletonAsset,
		JVector3<float>& effectorWorld,
		uint& ikCount,
		const uint ikNumber,
		const uint8 startJointRefIndex,
		const uint8 rootJointRefIndex,
		const std::vector<JMatrix4x4>& bindPose)noexcept
	{
		JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
		JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
		uint8 nowJointRefIndex = startJointRefIndex;
		const uint8 rootParentRefIndex = srcAvatar->jointReferenceParent[rootJointRefIndex];
		const uint8 rootIndex = srcAvatar->jointReference[rootJointRefIndex];
		//const XMMATRIX rootM = srcSkeleton->GetBindPose(rootIndex); 
		const XMMATRIX root = bindPose[rootIndex].LoadXM();

		ikCount = 0;
		while (nowJointRefIndex != rootParentRefIndex)
		{
			const uint nowJointIndex = srcAvatar->jointReference[nowJointRefIndex];
			//const XMMATRIX srcInitWM = srcSkeleton->GetBindPose(nowJointIndex); 
			const XMMATRIX srcInitWM = bindPose[nowJointIndex].LoadXM();
			const XMMATRIX srcInitLM = XMMatrixMultiply(srcInitWM, XMMatrixInverse(nullptr, root));

			updateData->ikJoint[ikNumber][ikCount].movedTransform.StoreXM(srcInitLM);
			updateData->ikJoint[ikNumber][ikCount].initTransform.StoreXM(srcInitLM);
			updateData->ikJoint[ikNumber][ikCount].jointIndex = nowJointIndex;
			updateData->ikJoint[ikNumber][ikCount].jointRefIndex = nowJointRefIndex;
			nowJointRefIndex = srcAvatar->jointReferenceParent[nowJointRefIndex];
			++ikCount;
		}

		updateData->ikJoint[ikNumber][ikCount].jointIndex = JSkeletonFixedData::incorrectJointIndex;
		updateData->ikJoint[ikNumber][ikCount].movedTransform = JMatrix4x4::Identity();
		updateData->ikJoint[ikNumber][ikCount].initTransform = JMatrix4x4::Identity();

		const XMVECTOR effectorWorldT = effectorWorld.ToXmV();
		const XMMATRIX effectorWorldM = XMMatrixTranslationFromVector(effectorWorldT);
		const XMMATRIX effectorWorldModM = XMMatrixMultiply(effectorWorldM, XMMatrixInverse(nullptr, root));
		XMVECTOR effectorWorldModS;
		XMVECTOR effectorWorldModQ;
		XMVECTOR effectorWorldModT;
		XMMatrixDecompose(&effectorWorldModS, &effectorWorldModQ, &effectorWorldModT, effectorWorldModM);

		effectorWorld = effectorWorldModT;
	}
	bool JAnimationPostProcessing::FindBindPoseIKValue(JAnimationUpdateData* updateData,
		JSkeletonAsset* srcSkeletonAsset,
		const JVector3<float>& effectorWorld,
		const uint ikNumber,
		const uint ikCount,
		const uint loopCount)noexcept
	{
		JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
		if (srcSkeleton == nullptr)
			return false;

		if (ikCount < 1)
			return false;

		const XMVECTOR effectorWT = effectorWorld.ToXmV();
		const XMVECTOR identityQ = XMVectorSet(0, 0, 0, 1);

		//updateData->ikJoint[ikNumber]
		for (uint i = 0; i < loopCount; ++i)
		{
			for (uint j = 1; j < ikCount; ++j)
			{
				const XMMATRIX endWorldM = updateData->ikJoint[ikNumber][0].movedTransform.LoadXM();
				const XMMATRIX movedWorldM = updateData->ikJoint[ikNumber][j].movedTransform.LoadXM();
				const XMMATRIX movedWorldParentM = updateData->ikJoint[ikNumber][j + 1].movedTransform.LoadXM();
				const XMMATRIX movedLocalM = XMMatrixMultiply(movedWorldM, XMMatrixInverse(nullptr, movedWorldParentM));

				XMVECTOR endWorldS;
				XMVECTOR endWorldQ;
				XMVECTOR endWorldT;
				XMMatrixDecompose(&endWorldS, &endWorldQ, &endWorldT, endWorldM);
				XMVECTOR movedWorldS;
				XMVECTOR movedWorldQ;
				XMVECTOR movedWorldT;
				XMMatrixDecompose(&movedWorldS, &movedWorldQ, &movedWorldT, movedWorldM);
				XMVECTOR movedWorldParentS;
				XMVECTOR movedWorldParentQ;
				XMVECTOR movedWorldParentT;
				XMMatrixDecompose(&movedWorldParentS, &movedWorldParentQ, &movedWorldParentT, movedWorldParentM);
				XMVECTOR movedLocalS;
				XMVECTOR movedLocalQ;
				XMVECTOR movedLocalT;
				XMMatrixDecompose(&movedLocalS, &movedLocalQ, &movedLocalT, movedLocalM);

				const XMVECTOR endLocalT = XMVectorSubtract(endWorldT, movedWorldT);
				const XMVECTOR effectorLocalT = XMVectorSubtract(effectorWT, movedWorldT);
				XMFLOAT3 distanceF;
				XMStoreFloat3(&distanceF, XMVector3Length(XMVectorSubtract(effectorLocalT, endLocalT)));
				if (distanceF.x <= minDistance)
					continue;

				const XMVECTOR endLocalTNor = XMVector3Normalize(endLocalT);
				const XMVECTOR effectorLocalTNor = XMVector3Normalize(effectorLocalT);

				const XMVECTOR anlgeV = XMVector3AngleBetweenVectors(endLocalTNor, effectorLocalTNor);
				XMFLOAT3 angleF;
				XMStoreFloat3(&angleF, anlgeV);
				float angle = angleF.x;
				const XMVECTOR axis = XMVector3Cross(endLocalTNor, effectorLocalTNor);
				const XMVECTOR deltaQ = XMQuaternionRotationAxis(axis, angle);
				const XMVECTOR resultAxis = XMVector3Cross(endLocalTNor, axis);
				const XMVECTOR constrainedQ = ConstrainAngle(deltaQ, axis).ToXmV();
				//const XMVECTOR globalConstrainedQ = XMQuaternionMultiply(XMQuaternionInverse(movedWorldParentQ),XMQuaternionMultiply(constrainedQ, movedWorldParentQ));
				//const XMVECTOR movedLocalModQ = XMQuaternionMultiply(globalConstrainedQ, movedLocalQ);
				//const XMVECTOR movedWorldModQ = XMQuaternionMultiply(movedLocalModQ, movedWorldParentQ);
				const XMVECTOR movedWorldModQ = XMQuaternionMultiply(movedWorldQ, constrainedQ);
				const XMMATRIX movedWorldModM = XMMatrixAffineTransformation(movedWorldS, identityQ, movedWorldModQ, movedWorldT);
				updateData->ikJoint[ikNumber][j].movedTransform.StoreXM(movedWorldModM);

				XMMATRIX preMovedWorldParentM = movedWorldM;
				for (int k = ((int)j) - 1; k >= 0; --k)
				{
					const XMMATRIX movedWorldParentModM = updateData->ikJoint[ikNumber][k + 1].movedTransform.LoadXM();
					const XMMATRIX movedWorldChildM = updateData->ikJoint[ikNumber][k].movedTransform.LoadXM();
					const XMMATRIX movedLocalChildM = XMMatrixMultiply(movedWorldChildM, XMMatrixInverse(nullptr, preMovedWorldParentM));
					const XMMATRIX movedWorldChildModM = XMMatrixMultiply(movedLocalChildM, movedWorldParentModM);
					updateData->ikJoint[ikNumber][k].movedTransform.StoreXM(movedWorldChildModM);
					preMovedWorldParentM = movedWorldChildM;
				}
			}
		}

		const uint8 rootIndex = updateData->ikJoint[ikNumber][ikCount - 1].jointIndex;
		const XMMATRIX rootWorld = srcSkeleton->GetBindPose(rootIndex);

		for (uint j = 0; j < ikCount; ++j)
		{
			const XMMATRIX movedWorldM = updateData->ikJoint[ikNumber][j].movedTransform.LoadXM();
			const XMMATRIX finalM = XMMatrixMultiply(movedWorldM, rootWorld);
			updateData->ikJoint[ikNumber][j].movedTransform.StoreXM(finalM);
		}
		return true;
	}
	JVector4<float> JAnimationPostProcessing::ConstrainAngle(const JVector4<float>& globalDeltaQ, const JVector3<float>& axis)noexcept
	{
		const JVector3<float> euler = JMathHelper::ToEulerAngle(globalDeltaQ);
		const float x = std::clamp(euler.x, -defaulAngleRestriction, defaulAngleRestriction);
		const float y = std::clamp(euler.y, -defaulAngleRestriction, defaulAngleRestriction);
		const float z = std::clamp(euler.z, -defaulAngleRestriction, defaulAngleRestriction);

		return JMathHelper::EulerToQuaternion(x, y, z);
	}
}


