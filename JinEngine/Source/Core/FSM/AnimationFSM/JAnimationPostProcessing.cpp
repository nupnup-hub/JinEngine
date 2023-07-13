#include"JAnimationPostProcessing.h"
#include"JAnimationUpdateData.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h" 
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h" 
#include"../../../Object/Resource/Skeleton/Avatar/JAvatar.h" 
#include<algorithm>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		bool JAnimationPostProcessing::CalculateBindPoseIK(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const J_AVATAR_JOINT start,
			const J_AVATAR_JOINT root,
			const uint ikNumber,
			const std::vector<DirectX::XMFLOAT3>& targetTranslation,
			const std::vector<DirectX::XMFLOAT4X4>& bindPose)noexcept
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

			XMFLOAT3 endEffectorf = targetTranslation[tarStIndex];
			StuffBindPoseData(updateData, srcSkeletonAsset, endEffectorf, updateData->ikCount[ikNumber], ikNumber, srcStRefIndex, srcRootRefIndex, bindPose);
			updateData->SetIKRate(ikNumber);
			bool res = FindBindPoseIKValue(updateData, srcSkeletonAsset, endEffectorf, ikNumber, updateData->ikCount[ikNumber], JAnimationFixedData::defualtBindPoseIkLoopCount);

			return res;
		}
		void JAnimationPostProcessing::StuffBindPoseData(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			DirectX::XMFLOAT3& effectorWorld,
			uint& ikCount,
			const uint ikNumber,
			const uint8 startJointRefIndex,
			const uint8 rootJointRefIndex,
			const std::vector<DirectX::XMFLOAT4X4>& bindPose)noexcept
		{
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			uint8 nowJointRefIndex = startJointRefIndex;
			const XMFLOAT4 identityQ = JMathHelper::QuaternionZero();
			const XMFLOAT4X4 identityM = JMathHelper::Identity4x4();
			const uint8 rootParentRefIndex = srcAvatar->jointReferenceParent[rootJointRefIndex];
			const uint8 rootIndex = srcAvatar->jointReference[rootJointRefIndex];
			//const XMMATRIX rootM = srcSkeleton->GetBindPose(rootIndex);
			const XMMATRIX rootM = XMLoadFloat4x4(&bindPose[rootIndex]);

			ikCount = 0;
			while (nowJointRefIndex != rootParentRefIndex)
			{
				const uint nowJointIndex = srcAvatar->jointReference[nowJointRefIndex];
				//const XMMATRIX srcInitWM = srcSkeleton->GetBindPose(nowJointIndex); 
				const XMMATRIX srcInitWM = XMLoadFloat4x4(&bindPose[nowJointIndex]);
				const XMMATRIX srcInitLM = XMMatrixMultiply(srcInitWM, XMMatrixInverse(nullptr, rootM));

				XMStoreFloat4x4(&updateData->ikJoint[ikNumber][ikCount].movedTransform, srcInitLM);
				XMStoreFloat4x4(&updateData->ikJoint[ikNumber][ikCount].initTransform, srcInitLM);
				updateData->ikJoint[ikNumber][ikCount].jointIndex = nowJointIndex;
				updateData->ikJoint[ikNumber][ikCount].jointRefIndex = nowJointRefIndex;
				nowJointRefIndex = srcAvatar->jointReferenceParent[nowJointRefIndex];
				++ikCount;
			}

			updateData->ikJoint[ikNumber][ikCount].jointIndex = JSkeletonFixedData::incorrectJointIndex;
			updateData->ikJoint[ikNumber][ikCount].movedTransform = identityM;
			updateData->ikJoint[ikNumber][ikCount].initTransform = identityM;

			const XMVECTOR effectorWorldT = XMLoadFloat3(&effectorWorld);
			const XMMATRIX effectorWorldM = XMMatrixTranslationFromVector(effectorWorldT);
			const XMMATRIX effectorWorldModM = XMMatrixMultiply(effectorWorldM, XMMatrixInverse(nullptr, rootM));
			XMVECTOR effectorWorldModS;
			XMVECTOR effectorWorldModQ;
			XMVECTOR effectorWorldModT;
			XMMatrixDecompose(&effectorWorldModS, &effectorWorldModQ, &effectorWorldModT, effectorWorldModM);

			XMStoreFloat3(&effectorWorld, effectorWorldModT);
		}
		bool JAnimationPostProcessing::FindBindPoseIKValue(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			const DirectX::XMFLOAT3& effectorWorld,
			const uint ikNumber,
			const uint ikCount,
			const uint loopCount)noexcept
		{
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			if (srcSkeleton == nullptr)
				return false;

			if (ikCount < 1)
				return false;

			const XMVECTOR effectorWT = XMLoadFloat3(&effectorWorld);
			const XMVECTOR identityQ = XMVectorSet(0, 0, 0, 1);

			//updateData->ikJoint[ikNumber]
			for (uint i = 0; i < loopCount; ++i)
			{
				for (uint j = 1; j < ikCount; ++j)
				{
					const XMMATRIX endWorldM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][0].movedTransform);
					const XMMATRIX movedWorldM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][j].movedTransform);
					const XMMATRIX movedWorldParentM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][j + 1].movedTransform);
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

					XMFLOAT3 axisF;
					XMFLOAT4 deltaQF;
					XMStoreFloat3(&axisF, axis);
					XMStoreFloat4(&deltaQF, deltaQ);
					const XMVECTOR constrainedQ = ConstrainAngle(deltaQF, axisF);
					//const XMVECTOR globalConstrainedQ = XMQuaternionMultiply(XMQuaternionInverse(movedWorldParentQ),XMQuaternionMultiply(constrainedQ, movedWorldParentQ));
					//const XMVECTOR movedLocalModQ = XMQuaternionMultiply(globalConstrainedQ, movedLocalQ);
					//const XMVECTOR movedWorldModQ = XMQuaternionMultiply(movedLocalModQ, movedWorldParentQ);
					const XMVECTOR movedWorldModQ = XMQuaternionMultiply(movedWorldQ, constrainedQ);
					const XMMATRIX movedWorldModM = XMMatrixAffineTransformation(movedWorldS, identityQ, movedWorldModQ, movedWorldT);
					XMStoreFloat4x4(&updateData->ikJoint[ikNumber][j].movedTransform, movedWorldModM);

					XMMATRIX preMovedWorldParentM = movedWorldM;
					for (int k = ((int)j) - 1; k >= 0; --k)
					{
						const XMMATRIX movedWorldParentModM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][k + 1].movedTransform);
						const XMMATRIX movedWorldChildM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][k].movedTransform);
						const XMMATRIX movedLocalChildM = XMMatrixMultiply(movedWorldChildM, XMMatrixInverse(nullptr, preMovedWorldParentM));
						const XMMATRIX movedWorldChildModM = XMMatrixMultiply(movedLocalChildM, movedWorldParentModM);
						XMStoreFloat4x4(&updateData->ikJoint[ikNumber][k].movedTransform, movedWorldChildModM);
						preMovedWorldParentM = movedWorldChildM;
					}
				}
			}

			const uint8 rootIndex = updateData->ikJoint[ikNumber][ikCount - 1].jointIndex;
			const XMMATRIX rootWorldM = srcSkeleton->GetBindPose(rootIndex);

			for (uint j = 0; j < ikCount; ++j)
			{
				const XMMATRIX movedWorldM = XMLoadFloat4x4(&updateData->ikJoint[ikNumber][j].movedTransform);
				const XMMATRIX finalM = XMMatrixMultiply(movedWorldM, rootWorldM);
				XMStoreFloat4x4(&updateData->ikJoint[ikNumber][j].movedTransform, finalM);
			}
			return true;
		}
		DirectX::XMVECTOR JAnimationPostProcessing::ConstrainAngle(const DirectX::XMFLOAT4& globalDeltaQ, const DirectX::XMFLOAT3& axis)noexcept
		{
			XMFLOAT3 euler = JMathHelper::ToEulerAngle(globalDeltaQ);

			float x = std::clamp(euler.x, -defaulAngleRestriction, defaulAngleRestriction);
			float y = std::clamp(euler.y, -defaulAngleRestriction, defaulAngleRestriction);
			float z = std::clamp(euler.z, -defaulAngleRestriction, defaulAngleRestriction);


			XMFLOAT4 constrainedQ = JMathHelper::EulerToQuaternion(x, y, z);
			return XMLoadFloat4(&constrainedQ);
		}
	}
}
