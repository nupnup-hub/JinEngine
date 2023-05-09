#include"JAnimationRetargeting.h"
#include"JAnimationUpdateData.h"
#include"JAnimationPostProcessing.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Object/Resource/Skeleton/Avatar/JAvatar.h"
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		bool JAnimationRetargeting::CalculateAdditionalBindPose(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind)noexcept
		{
			std::vector<DirectX::XMFLOAT3>bindTarLocalT(JSkeletonFixedData::maxJointCount);
			std::vector<DirectX::XMFLOAT3>bindTarWorldT(JSkeletonFixedData::maxJointCount);

			const XMFLOAT4 idenQF = JMathHelper::QuaternionZero();
			const XMVECTOR idenQV = XMVectorSet(0, 0, 0, 1);

			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();

			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			const uint8 srcJointCount = srcSkeleton->GetJointCount();
			const uint8 tarJointCount = tarSkeleton->GetJointCount();

			StuffTargetTranslation(updateData, srcSkeletonAsset, tarSkeletonAsset, bindTarLocalT, bindTarWorldT);
			ReBuildTargetTranslation(updateData, srcSkeletonAsset, tarSkeletonAsset, bindTarLocalT, bindTarWorldT, 0);
			GetBindPoseAdditionalTransform(updateData, srcSkeletonAsset, tarSkeletonAsset, bindTarWorldT, additionalBind);
			return true;
		}
		void JAnimationRetargeting::StuffTargetTranslation(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<DirectX::XMFLOAT3>& bindTargetLocalT,
			std::vector<DirectX::XMFLOAT3>& bindTargetWorldT)noexcept
		{
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();

			for (uint i = 0; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
			{
				const uint8 srcIndex = srcAvatar->jointReference[i];
				const uint8 tarIndex = tarAvatar->jointReference[i];

				if (srcIndex == JSkeletonFixedData::incorrectJointIndex || tarIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				srcAvatar->jointInterpolation[srcIndex].isUpdated = true;
				const XMMATRIX srcWorldBindM = srcSkeleton->GetBindPose(srcIndex);
				const XMMATRIX tarWorldBindM = tarSkeleton->GetBindPose(tarIndex);

				XMVECTOR bindSrcS;
				XMVECTOR bindSrcQ;
				XMVECTOR bindSrcT;
				XMVECTOR bindTarS;
				XMVECTOR bindTarQ;
				XMVECTOR bindTarT;

				XMMatrixDecompose(&bindSrcS, &bindSrcQ, &bindSrcT, srcWorldBindM);
				XMMatrixDecompose(&bindTarS, &bindTarQ, &bindTarT, tarWorldBindM);

				XMStoreFloat3(&bindTargetWorldT[tarIndex], bindTarT);

				if (i != JSkeletonFixedData::rootJointIndex)
				{
					const uint8 parentRefIndex = tarAvatar->jointReferenceParent[i];
					const uint8 srcParentIndex = srcAvatar->jointReference[parentRefIndex];
					const uint8 tarParentIndex = tarAvatar->jointReference[parentRefIndex];

					const XMMATRIX srcParentWorldBindM = srcSkeleton->GetBindPose(srcParentIndex);
					const XMMATRIX tarParentWorldBindM = tarSkeleton->GetBindPose(tarParentIndex);

					XMVECTOR bindParentSrcS;
					XMVECTOR bindParentSrcQ;
					XMVECTOR bindParentSrcT;
					XMVECTOR bindParentTarS;
					XMVECTOR bindParentTarQ;
					XMVECTOR bindParentTarT;

					XMMatrixDecompose(&bindParentSrcS, &bindParentSrcQ, &bindParentSrcT, srcParentWorldBindM);
					XMMatrixDecompose(&bindParentTarS, &bindParentTarQ, &bindParentTarT, tarParentWorldBindM);

					const XMVECTOR bindSrcLocalT = XMVectorSubtract(bindSrcT, bindParentSrcT);
					const XMVECTOR bindTarLocalT = XMVectorSubtract(bindTarT, bindParentTarT);
					const XMVECTOR bindSrcLocalTDistanceV = XMVector3Length(bindSrcLocalT);
					const XMVECTOR bindTarLocalTDistanceV = XMVector3Length(bindTarLocalT);
					XMFLOAT3 bindSrcLocalTDistance;
					XMFLOAT3 bindTarLocalTDistance;
					XMStoreFloat3(&bindSrcLocalTDistance, bindSrcLocalTDistanceV);
					XMStoreFloat3(&bindTarLocalTDistance, bindTarLocalTDistanceV);
					float lengthRate = bindSrcLocalTDistance.x / bindTarLocalTDistance.x;

					XMStoreFloat3(&bindTargetLocalT[tarIndex], XMVectorScale(bindTarLocalT, lengthRate));
				}
				else
				{
					const XMVECTOR bindSrcLocalTDistanceV = XMVector3Length(bindSrcT);
					const XMVECTOR bindTarLocalTDistanceV = XMVector3Length(bindTarT);
					XMFLOAT3 bindSrcLocalTDistance;
					XMFLOAT3 bindTarLocalTDistance;
					XMStoreFloat3(&bindSrcLocalTDistance, bindSrcLocalTDistanceV);
					XMStoreFloat3(&bindTarLocalTDistance, bindTarLocalTDistanceV);
					float lengthRate = bindSrcLocalTDistance.x / bindTarLocalTDistance.x;
					XMStoreFloat3(&bindTargetLocalT[tarIndex], XMVectorScale(bindTarT, lengthRate));
				}
			}
		}
		void JAnimationRetargeting::ReBuildTargetTranslation(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<DirectX::XMFLOAT3>& bindTargetLocalT,
			std::vector<DirectX::XMFLOAT3>& bindTargetWorldT,
			uint8 refIndex)noexcept
		{
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();
			const uint8 srcIndex = srcAvatar->jointReference[refIndex];
			const uint8 tarIndex = tarAvatar->jointReference[refIndex];

			if (srcIndex == JSkeletonFixedData::incorrectJointIndex ||
				tarIndex == JSkeletonFixedData::incorrectJointIndex)
				return;

			auto data = JAvatar::jointReferenceChildren.find(refIndex);
			if (data == JAvatar::jointReferenceChildren.end())
				return;

			for (const auto& childRefIndex : data->second)
			{
				const uint8 srcChildIndex = srcAvatar->jointReference[childRefIndex];
				const uint8 tarChildIndex = tarAvatar->jointReference[childRefIndex];

				if (srcChildIndex == JSkeletonFixedData::incorrectJointIndex ||
					tarChildIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				const XMVECTOR childLocal = XMLoadFloat3(&bindTargetLocalT[tarChildIndex]);
				const XMVECTOR parentWorld = XMLoadFloat3(&bindTargetWorldT[tarIndex]);
				const XMVECTOR childWorld = XMVectorAdd(childLocal, parentWorld);
				XMStoreFloat3(&bindTargetWorldT[tarChildIndex], childWorld);

				ReBuildTargetTranslation(updateData, srcSkeletonAsset, tarSkeletonAsset, bindTargetLocalT, bindTargetWorldT, childRefIndex);
			}
		}
		void JAnimationRetargeting::GetBindPoseAdditionalTransform(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const std::vector<DirectX::XMFLOAT3>& tarModTranslation,
			std::vector<JAnimationAdditionalBind>& additionalBind)noexcept
		{
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();

			size_t srcGuid = srcSkeletonAsset->GetGuid();
			size_t tarGuid = tarSkeletonAsset->GetGuid();

			if (srcGuid == tarGuid)
				return;

			const XMFLOAT4X4 iden = JMathHelper::Identity4x4();
			const uint8 jointCount = srcSkeleton->GetJointCount();
			std::vector<DirectX::XMFLOAT4X4> bindPose(jointCount);
			for (uint i = 0; i < jointCount; ++i)
				XMStoreFloat4x4(&bindPose[i], srcSkeleton->GetBindPose(i));

			updateData->EnterCalculateIK();
			JAnimationPostProcessing::CalculateBindPoseIK(updateData, srcSkeletonAsset, tarSkeletonAsset, J_AVATAR_JOINT::LEFT_HAND, J_AVATAR_JOINT::LEFT_SHOULDER, 0, tarModTranslation, bindPose);
			JAnimationPostProcessing::CalculateBindPoseIK(updateData, srcSkeletonAsset, tarSkeletonAsset, J_AVATAR_JOINT::RIGHT_HAND, J_AVATAR_JOINT::RIGHT_SHOULDER, 1, tarModTranslation, bindPose);
			JAnimationPostProcessing::CalculateBindPoseIK(updateData, srcSkeletonAsset, tarSkeletonAsset, J_AVATAR_JOINT::LEFT_LOWER_LEG, J_AVATAR_JOINT::LEFT_UPPER_LEG, 2, tarModTranslation, bindPose);
			JAnimationPostProcessing::CalculateBindPoseIK(updateData, srcSkeletonAsset, tarSkeletonAsset, J_AVATAR_JOINT::RIGHT_LOWER_LEG, J_AVATAR_JOINT::RIGHT_UPPER_LEG, 3, tarModTranslation, bindPose);
			StuffBindPoseIKResult(updateData, srcSkeletonAsset, tarSkeletonAsset, additionalBind, bindPose);

			AdjustJoint(updateData, srcSkeletonAsset, tarSkeletonAsset, bindPose);
			//StuffBindPoseIKResult(updateData, srcSkeletonAsset, tarSkeletonAsset, additionalBind, bindPose);

			for (uint i = 1; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
			{
				const uint8 tarIndex = tarAvatar->jointReference[i];
				const uint8 srcIndex = srcAvatar->jointReference[i];

				if (tarIndex == JSkeletonFixedData::incorrectJointIndex ||
					srcIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				const XMMATRIX modM = XMLoadFloat4x4(&bindPose[srcIndex]);
				const XMMATRIX srcM = srcSkeleton->GetBindPose(srcIndex);
				const XMMATRIX toModM = XMMatrixMultiply(modM, XMMatrixInverse(nullptr, srcM));
				XMStoreFloat4x4(&additionalBind[i].transform, toModM);
			}
		}
		void JAnimationRetargeting::StuffBindPoseIKResult(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();
			size_t srcGuid = srcSkeletonAsset->GetGuid();
			size_t tarGuid = tarSkeletonAsset->GetGuid();

			for (uint i = 0; i < JAnimationFixedData::defaultIKSlotCount; ++i)
			{
				if (updateData->ikCount[i] > 0)
				{
					for (uint j = 0; j < updateData->ikCount[i]; ++j)
					{
						const uint8 jointIndex = updateData->ikJoint[i][j].jointIndex;
						const uint8 jointRefIndex = updateData->ikJoint[i][j].jointRefIndex;
						const XMMATRIX modM = XMLoadFloat4x4(&updateData->ikJoint[i][j].movedTransform);
						const XMMATRIX srcM = srcSkeleton->GetBindPose(jointIndex);
						const XMMATRIX toModM = XMMatrixMultiply(modM, XMMatrixInverse(nullptr, srcM));
						XMStoreFloat4x4(&additionalBind[jointRefIndex].transform, toModM);
						XMStoreFloat4x4(&modBindPose[jointIndex], modM);
						additionalBind[jointRefIndex].isIKModified = true;
					}
				}
			}

			for (uint i = 1; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
			{
				const uint8 tarIndex = tarAvatar->jointReference[i];
				const uint8 srcIndex = srcAvatar->jointReference[i];

				if (tarIndex == JSkeletonFixedData::incorrectJointIndex ||
					srcIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				const uint8 parentRef = JAvatar::jointReferenceParent[i];
				const uint8 tarParentIndex = tarAvatar->jointReference[parentRef];
				const uint8 srcParentIndex = srcAvatar->jointReference[parentRef];

				if (additionalBind[parentRef].isIKModified && !additionalBind[i].isIKModified)
				{
					const XMMATRIX srcParentAdditionalM = XMLoadFloat4x4(&additionalBind[parentRef].transform);
					const XMMATRIX srcParentWorldBind = srcSkeleton->GetBindPose(srcParentIndex);
					const XMMATRIX srcParentWorldBindMod = XMMatrixMultiply(srcParentAdditionalM, srcParentWorldBind);

					const XMMATRIX srcWorldBind = srcSkeleton->GetBindPose(srcIndex);
					const XMMATRIX srcLocalBind = XMMatrixMultiply(srcWorldBind, XMMatrixInverse(nullptr, srcParentWorldBind));
					const XMMATRIX srcWorldBindMod = XMMatrixMultiply(srcLocalBind, srcParentWorldBindMod);
					const XMMATRIX srcAdditionalM = XMMatrixMultiply(srcWorldBindMod, XMMatrixInverse(nullptr, srcWorldBind));
					XMStoreFloat4x4(&additionalBind[i].transform, srcAdditionalM);
					XMStoreFloat4x4(&modBindPose[srcIndex], srcWorldBindMod);
					additionalBind[i].isIKModified = true;
				}
			}
		}
		void JAnimationRetargeting::AdjustJoint(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			const XMVECTOR idenQ = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR offsetDir = XMVectorSet(0, 0, 1, 0);
			const std::vector<uint8> restrictIndex
			{
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::JAW)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_EYE)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_EYE)->second,
			};
			const std::unordered_map<uint8, std::vector<uint8>> multiJointOrder
			{
				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_HAND)->second,
					{
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_MIDDLE_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_THUMB_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_LITTLE_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_INDEX_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_RING_PROXIMAL)->second,
					}
				},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_HAND)->second,
					{
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_MIDDLE_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_THUMB_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_LITTLE_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_INDEX_PROXIMAL)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_RING_PROXIMAL)->second,
					}
				},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::HIPS)->second,
					{
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::SPINE)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_UPPER_LEG)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_UPPER_LEG)->second,
					}
				},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::UPPER_CHEST)->second,
					{
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::NECK)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_SHOULDER)->second,
						JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_SHOULDER)->second,
					}
				}
			};

			const uint8 handL = JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_HAND)->second;
			const uint8 handR = JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_HAND)->second;

			const uint8 srcJointCount = (uint8)srcSkeleton->GetJointCount();
			const uint8 tarJointCount = (uint8)tarSkeleton->GetJointCount();
			const uint8 restrictIndexSize = (uint8)restrictIndex.size();

			for (uint i = 0; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
			{
				const uint8 nowRefIndex = i;
				const uint8 srcIndex = srcAvatar->jointReference[nowRefIndex];
				const uint8 tarIndex = tarAvatar->jointReference[nowRefIndex];

				if (srcIndex == JSkeletonFixedData::incorrectJointIndex || tarIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				bool isRestrictIndex = false;
				for (uint j = 0; j < restrictIndexSize; ++j)
				{
					if (restrictIndex[j] == nowRefIndex)
					{
						isRestrictIndex = true;
						break;
					}
				}
				if (isRestrictIndex)
					continue;

				auto avatarChildren = JAvatar::jointReferenceChildren.find(nowRefIndex);
				if (avatarChildren == JAvatar::jointReferenceChildren.end())
				{
					bool hasSrcChild = false;
					bool hasTarChild = false;
					uint8 srcChildIndex;
					uint8 tarChildIndex;
					for (uint j = srcIndex + 1; j < srcJointCount; ++j)
					{
						if (srcAvatar->jointBackReferenceMap[j].allottedParentRefIndex == nowRefIndex)
						{
							srcChildIndex = j;
							hasSrcChild = true;
							break;
						}
					}
					for (uint j = tarIndex + 1; j < tarJointCount; ++j)
					{
						if (tarAvatar->jointBackReferenceMap[j].allottedParentRefIndex == nowRefIndex)
						{
							tarChildIndex = j;
							hasTarChild = true;
							break;
						}
					}

					if (!hasTarChild || !hasSrcChild)
						continue;

					RotateJointByWorldQuaternionGap(srcSkeletonAsset, tarSkeletonAsset, srcIndex, tarIndex, srcChildIndex, tarChildIndex, modBindPose);
				}
				else if (avatarChildren->second.size() > 2)
				{
					auto avatarChildrenOrder = multiJointOrder.find(nowRefIndex);
					if (avatarChildrenOrder == multiJointOrder.end())
						continue;

					const uint8 srcFirstChildIndex = srcAvatar->jointReference[avatarChildrenOrder->second[0]];
					const uint8 tarFirstChildIndex = tarAvatar->jointReference[avatarChildrenOrder->second[0]];

					RotateJointByWorldQuaternionGap(srcSkeletonAsset, tarSkeletonAsset, srcIndex, tarIndex, srcFirstChildIndex, tarFirstChildIndex, modBindPose);

					const uint8 childrenSize = (uint8)avatarChildrenOrder->second.size();
					for (uint j = 0; j < childrenSize; ++j)
					{
						const uint8 nowChildRefIndex = avatarChildrenOrder->second[j];
						const uint8 srcChildIndex = srcAvatar->jointReference[nowChildRefIndex];
						const uint8 tarChildIndex = tarAvatar->jointReference[nowChildRefIndex];

						if (srcChildIndex == JSkeletonFixedData::incorrectJointIndex ||
							tarChildIndex == JSkeletonFixedData::incorrectJointIndex)
							continue;

						TransformJointByWorldQuaternionGap(srcSkeletonAsset, tarSkeletonAsset, srcIndex, tarIndex, srcChildIndex, tarChildIndex, modBindPose);
					}
				}
				else
				{
					//if (nowRefIndex == JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_FOOT)->second ||
					//	nowRefIndex == JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_FOOT)->second)
					//	continue;

					const uint8 childRefIndex = avatarChildren->second[0];
					const uint8 srcChildIndex = srcAvatar->jointReference[childRefIndex];
					const uint8 tarChildIndex = tarAvatar->jointReference[childRefIndex];
					if (srcChildIndex == JSkeletonFixedData::incorrectJointIndex ||
						tarChildIndex == JSkeletonFixedData::incorrectJointIndex)
						continue;

					RotateJointByWorldQuaternionGap(srcSkeletonAsset, tarSkeletonAsset, srcIndex, tarIndex, srcChildIndex, tarChildIndex, modBindPose);
				}
			}
		}
		void JAnimationRetargeting::RotateJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const uint8 srcIndex,
			const uint8 tarIndex,
			const uint8 srcChildIndex,
			const uint8 tarChildIndex,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			const XMVECTOR idenQ = XMVectorSet(0, 0, 0, 1);

			const XMMATRIX srcWorldModBind = XMLoadFloat4x4(&modBindPose[srcIndex]);
			const XMMATRIX srcChildWorldModBind = XMLoadFloat4x4(&modBindPose[srcChildIndex]);

			const XMMATRIX tarWorldBind = tarSkeleton->GetBindPose(tarIndex);
			const XMMATRIX tarChildWorldBind = tarSkeleton->GetBindPose(tarChildIndex);

			XMVECTOR srcWorldT;
			XMVECTOR srcWorldQ;
			XMVECTOR srcWorldS;
			XMMatrixDecompose(&srcWorldS, &srcWorldQ, &srcWorldT, srcWorldModBind);
			XMVECTOR srcChildWorldT;
			XMVECTOR srcChildWorldQ;
			XMVECTOR srcChildWorldS;
			XMMatrixDecompose(&srcChildWorldS, &srcChildWorldQ, &srcChildWorldT, srcChildWorldModBind);
			XMVECTOR tarWorldT;
			XMVECTOR tarWorldQ;
			XMVECTOR tarWorldS;
			XMMatrixDecompose(&tarWorldS, &tarWorldQ, &tarWorldT, tarWorldBind);
			XMVECTOR tarChildWorldT;
			XMVECTOR tarChildWorldQ;
			XMVECTOR tarChildWorldS;
			XMMatrixDecompose(&tarChildWorldS, &tarChildWorldQ, &tarChildWorldT, tarChildWorldBind);

			const XMVECTOR srcLocalDir = XMVectorSubtract(srcChildWorldT, srcWorldT);
			const XMVECTOR tarLocalDir = XMVectorSubtract(tarChildWorldT, tarWorldT);

			const XMVECTOR srcLocalDirNor = XMVector3Normalize(srcLocalDir);
			const XMVECTOR tarLocalDirNor = XMVector3Normalize(tarLocalDir);

			const XMVECTOR anlgeV = XMVector3AngleBetweenVectors(srcLocalDirNor, tarLocalDirNor);
			XMFLOAT3 angleF;
			XMStoreFloat3(&angleF, anlgeV);
			float angle = angleF.x;
			const XMVECTOR axis = XMVector3Cross(srcLocalDirNor, tarLocalDirNor);
			const XMVECTOR deltaQ = XMQuaternionRotationAxis(axis, angle);
			const XMVECTOR srcWorldModQ = XMQuaternionMultiply(srcWorldQ, deltaQ);
			const XMMATRIX srcWorldFinalBind = XMMatrixAffineTransformation(srcWorldS, idenQ, srcWorldModQ, srcWorldT);

			XMStoreFloat4x4(&modBindPose[srcIndex], srcWorldFinalBind);
			UpdateParentBindPose(srcSkeletonAsset, modBindPose, srcWorldModBind, srcIndex);
		}
		void JAnimationRetargeting::TransformJointByWorldQuaternionGap(JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			const uint8 srcIndex,
			const uint8 tarIndex,
			const uint8 srcChildIndex,
			const uint8 tarChildIndex,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			const XMVECTOR idenQ = XMVectorSet(0, 0, 0, 1);

			const XMMATRIX srcWorldModBind = XMLoadFloat4x4(&modBindPose[srcIndex]);
			const XMMATRIX srcChildWorldModBind = XMLoadFloat4x4(&modBindPose[srcChildIndex]);
			const XMMATRIX srcChildLocalModBind = XMMatrixMultiply(srcChildWorldModBind, XMMatrixInverse(nullptr, srcWorldModBind));

			const XMMATRIX tarWorldBind = tarSkeleton->GetBindPose(tarIndex);
			const XMMATRIX tarChildWorldBind = tarSkeleton->GetBindPose(tarChildIndex);

			XMVECTOR srcWorldT;
			XMVECTOR srcWorldQ;
			XMVECTOR srcWorldS;
			XMMatrixDecompose(&srcWorldS, &srcWorldQ, &srcWorldT, srcWorldModBind);
			XMVECTOR srcChildWorldT;
			XMVECTOR srcChildWorldQ;
			XMVECTOR srcChildWorldS;
			XMMatrixDecompose(&srcChildWorldS, &srcChildWorldQ, &srcChildWorldT, srcChildWorldModBind);
			XMVECTOR tarWorldT;
			XMVECTOR tarWorldQ;
			XMVECTOR tarWorldS;
			XMMatrixDecompose(&tarWorldS, &tarWorldQ, &tarWorldT, tarWorldBind);
			XMVECTOR tarChildWorldT;
			XMVECTOR tarChildWorldQ;
			XMVECTOR tarChildWorldS;
			XMMatrixDecompose(&tarChildWorldS, &tarChildWorldQ, &tarChildWorldT, tarChildWorldBind);

			const XMVECTOR srcLocalDir = XMVectorSubtract(srcChildWorldT, srcWorldT);
			const XMVECTOR tarLocalDir = XMVectorSubtract(tarChildWorldT, tarWorldT);

			const XMVECTOR srcLocalDirNor = XMVector3Normalize(srcLocalDir);
			const XMVECTOR tarLocalDirNor = XMVector3Normalize(tarLocalDir);

			const XMVECTOR anlgeV = XMVector3AngleBetweenVectors(srcLocalDirNor, tarLocalDirNor);
			XMFLOAT3 angleF;
			XMStoreFloat3(&angleF, anlgeV);
			float angle = angleF.x;
			const XMVECTOR axis = XMVector3Cross(srcLocalDirNor, tarLocalDirNor);
			const XMVECTOR deltaQ = XMQuaternionRotationAxis(axis, angle);
			const XMVECTOR srcWorldModQ = XMQuaternionMultiply(srcWorldQ, deltaQ);
			const XMMATRIX srcWorldFinalBind = XMMatrixAffineTransformation(srcWorldS, idenQ, srcWorldModQ, srcWorldT);
			const XMMATRIX srcChildFinalBind = XMMatrixMultiply(srcChildLocalModBind, srcWorldFinalBind);

			XMStoreFloat4x4(&modBindPose[srcChildIndex], srcChildFinalBind);
			UpdateParentBindPose(srcSkeletonAsset, modBindPose, srcChildWorldModBind, srcChildIndex);
		}
		void JAnimationRetargeting::UpdateParentBindPose(JSkeletonAsset* skeletonAsset,
			std::vector<DirectX::XMFLOAT4X4>& bindPose,
			const DirectX::XMMATRIX preParentBindPose,
			const uint8 index)
		{
			if (index == JSkeletonFixedData::incorrectJointIndex)
				return;

			JAvatar* avatar = skeletonAsset->GetAvatar().Get();
			JSkeleton* skeleton = skeletonAsset->GetSkeleton().Get();
			const uint8 jointCount = skeleton->GetJointCount();
			for (uint i = index + 1; i < jointCount; ++i)
			{
				if (skeleton->GetJointParentIndex(i) == index)
				{
					const XMMATRIX oriWorld = XMLoadFloat4x4(&bindPose[i]);
					const XMMATRIX oriLocal = XMMatrixMultiply(oriWorld, XMMatrixInverse(nullptr, preParentBindPose));

					const XMMATRIX modParent = XMLoadFloat4x4(&bindPose[index]);
					const XMMATRIX modWorld = XMMatrixMultiply(oriLocal, modParent);
					XMStoreFloat4x4(&bindPose[i], modWorld);
					UpdateParentBindPose(skeletonAsset, bindPose, oriWorld, i);
				}
			}
			/*for (const auto& childRef : child->second)
			{
				const uint8 childIndex = avatar->jointReference[childRef];
				if (childIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				const XMMATRIX oriParent = XMLoadFloat4x4(&bindPose[childIndex]);
				const XMMATRIX oriWorld = XMLoadFloat4x4(&bindPose[index]);
				const XMMATRIX oriLocal = XMMatrixMultiply(oriWorld, XMMatrixInverse(nullptr, oriParent));

				const XMMATRIX modParent = XMLoadFloat4x4(&bindPose[index]);
				const XMMATRIX modWorld = XMMatrixMultiply(oriLocal, modParent);
				XMStoreFloat4x4(&bindPose[childIndex], modWorld);
				UpdateParentBindPose(skeletonAsset, bindPose, childRef);
			}*/
			/*for (uint i = index + 1; i < jointCount; ++i)
			{
				if (skeleton->GetJointParentIndex(i) == index)
				{
					const XMMATRIX oriParent = skeleton->GetBindPose(index);
					const XMMATRIX oriWorld = skeleton->GetBindPose(i);
					const XMMATRIX oriLocal = XMMatrixMultiply(oriWorld, XMMatrixInverse(nullptr, oriParent));

					const XMMATRIX modParent = XMLoadFloat4x4(&bindPose[index]);
					const XMMATRIX modWorld = XMMatrixMultiply(oriLocal, modParent);
					XMStoreFloat4x4(&bindPose[i], modWorld);
					UpdateParentBindPose(skeletonAsset, bindPose, i);
				}
			}*/
		}

		void JAnimationRetargeting::DebugQGapPrint00(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{

			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();

			std::vector<std::vector<uint8>>refIndex
			{
				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::HIPS)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_UPPER_LEG)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::HIPS)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_UPPER_LEG)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::HIPS)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::SPINE)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::CHEST)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::UPPER_CHEST)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::UPPER_CHEST)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_SHOULDER)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::UPPER_CHEST)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_SHOULDER)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_SHOULDER)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_UPPER_ARM)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_LOWER_ARM)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_HAND)->second},

				{JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_SHOULDER)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_UPPER_ARM)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_LOWER_ARM)->second,
				JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_HAND)->second},
			};
			const XMVECTOR idenQ = XMVectorSet(0, 0, 0, 1);

			for (uint i = 0; i < refIndex.size(); ++i)
			{
				for (uint j = 0; j < refIndex[i].size() - 1; ++j)
				{
					const uint8 nowRefIndex = refIndex[i][j];
					const uint8 srcIndex = srcAvatar->jointReference[nowRefIndex];
					const uint8 tarIndex = tarAvatar->jointReference[nowRefIndex];

					if (srcIndex == JSkeletonFixedData::incorrectJointIndex ||
						tarIndex == JSkeletonFixedData::incorrectJointIndex)
						continue;

					const uint8 childRefIndex = refIndex[i][j + 1];
					const uint8 srcChildIndex = srcAvatar->jointReference[childRefIndex];
					const uint8 tarChildIndex = tarAvatar->jointReference[childRefIndex];
					if (srcChildIndex == JSkeletonFixedData::incorrectJointIndex ||
						tarChildIndex == JSkeletonFixedData::incorrectJointIndex)
						continue;

					const XMMATRIX srcWorldModBind = XMLoadFloat4x4(&modBindPose[srcIndex]);
					const XMMATRIX srcChildWorldModBind = XMLoadFloat4x4(&modBindPose[srcChildIndex]);

					const XMMATRIX tarWorldBind = tarSkeleton->GetBindPose(tarIndex);
					const XMMATRIX tarChildWorldBind = tarSkeleton->GetBindPose(tarChildIndex);

					XMVECTOR srcWorldT;
					XMVECTOR srcWorldQ;
					XMVECTOR srcWorldS;
					XMMatrixDecompose(&srcWorldS, &srcWorldQ, &srcWorldT, srcWorldModBind);
					XMVECTOR srcChildWorldT;
					XMVECTOR srcChildWorldQ;
					XMVECTOR srcChildWorldS;
					XMMatrixDecompose(&srcChildWorldS, &srcChildWorldQ, &srcChildWorldT, srcChildWorldModBind);
					XMVECTOR tarWorldT;
					XMVECTOR tarWorldQ;
					XMVECTOR tarWorldS;
					XMMatrixDecompose(&tarWorldS, &tarWorldQ, &tarWorldT, tarWorldBind);
					XMVECTOR tarChildWorldT;
					XMVECTOR tarChildWorldQ;
					XMVECTOR tarChildWorldS;
					XMMatrixDecompose(&tarChildWorldS, &tarChildWorldQ, &tarChildWorldT, tarChildWorldBind);

					const XMVECTOR srcLocalDir = XMVectorSubtract(srcChildWorldT, srcWorldT);
					const XMVECTOR tarLocalDir = XMVectorSubtract(tarChildWorldT, tarWorldT);

					const XMVECTOR srcLocalDirNor = XMVector3Normalize(srcLocalDir);
					const XMVECTOR tarLocalDirNor = XMVector3Normalize(tarLocalDir);

					const XMVECTOR anlgeV = XMVector3AngleBetweenVectors(srcLocalDirNor, tarLocalDirNor);
					XMFLOAT3 angleF;
					XMStoreFloat3(&angleF, anlgeV);
					float angle = angleF.x;
					const XMVECTOR axis = XMVector3Cross(srcLocalDirNor, tarLocalDirNor);
					const XMVECTOR deltaQ = XMQuaternionRotationAxis(axis, angle);
					const XMVECTOR srcWorldModQ = XMQuaternionMultiply(deltaQ, srcWorldQ);
					const XMMATRIX srcWorldFinalBind = XMMatrixAffineTransformation(srcWorldS, idenQ, srcWorldModQ, srcWorldT);

					//XMStoreFloat4x4(&modBindPose[srcIndex], srcWorldFinalBind);
					//JDebugTextOut::PrintWstr(JCUtil::StrToWstr("D root: " + srcSkeleton->GetJointName(srcIndex)));
					//JDebugTextOut::PrintWstr(JCUtil::StrToWstr("Child: " + srcSkeleton->GetJointName(srcChildIndex)));
					//JDebugTextOut::PrintQuaternion(deltaQ, L"deltaQ");
					//JDebugTextOut::PrintVector(srcLocalDir, L"srcLocalDir");
					//JDebugTextOut::PrintVector(tarLocalDir, L"tarLocalDir");
					//JDebugTextOut::PrintEnter(1);
					//UpdateParentBindPose(srcSkeletonAsset, modBindPose, srcWorldModBind, srcIndex);
				}
			}
		}
		void JAnimationRetargeting::DebugQGapPrint01(JAnimationUpdateData* updateData,
			JSkeletonAsset* srcSkeletonAsset,
			JSkeletonAsset* tarSkeletonAsset,
			std::vector<JAnimationAdditionalBind>& additionalBind,
			std::vector<DirectX::XMFLOAT4X4>& modBindPose)noexcept
		{
			//Test
			JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton().Get();
			JSkeleton* tarSkeleton = tarSkeletonAsset->GetSkeleton().Get();
			JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar().Get();
			JAvatar* tarAvatar = tarSkeletonAsset->GetAvatar().Get();

			const XMVECTOR idenQ = XMVectorSet(0, 0, 0, 1);
			const XMFLOAT4 y90F = JMathHelper::EulerToQuaternion(0, 90, 45);
			const XMVECTOR y90V = XMLoadFloat4(&y90F);
			const XMMATRIX y90M = XMMatrixRotationQuaternion(y90V);

			const uint8 leftShoulderRef = JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::LEFT_SHOULDER)->second;
			const uint8 rightShoulderRef = JAvatar::jointReferenceMap.find(J_AVATAR_JOINT::RIGHT_SHOULDER)->second;

			const uint8 leftShoulderParentRef = JAvatar::jointReferenceParent[leftShoulderRef];
			const uint8 rightShoulderParentRef = JAvatar::jointReferenceParent[rightShoulderRef];

			const uint8 leftShoulderParentIndex = srcAvatar->jointReference[leftShoulderParentRef];
			const uint8 rightShoulderParentIndex = srcAvatar->jointReference[rightShoulderParentRef];

			const uint8 leftShoulderIndex = srcAvatar->jointReference[leftShoulderRef];
			const uint8 rightShoulderIndex = srcAvatar->jointReference[rightShoulderRef];

			const XMVECTOR leftShoulderParentQ = srcSkeleton->GetBindQuaternion(leftShoulderParentIndex);
			const XMVECTOR rightShoulderParentQ = srcSkeleton->GetBindQuaternion(rightShoulderParentIndex);

			const XMMATRIX leftShoulder = srcSkeleton->GetBindPose(leftShoulderIndex);
			const XMMATRIX rightShoulder = srcSkeleton->GetBindPose(rightShoulderIndex);

			XMVECTOR leftT;
			XMVECTOR leftQ;
			XMVECTOR leftS;
			XMVECTOR rightT;
			XMVECTOR rightQ;
			XMVECTOR rightS;
			XMMatrixDecompose(&leftS, &leftQ, &leftT, leftShoulder);
			XMMatrixDecompose(&rightS, &rightQ, &rightT, rightShoulder);

			const XMVECTOR leftModQ = XMQuaternionMultiply(leftQ, y90V);
			const XMVECTOR rightModQ = XMQuaternionMultiply(rightQ, y90V);

			const XMMATRIX leftShoulderMod = XMMatrixAffineTransformation(leftS, idenQ, leftModQ, leftT);
			const XMMATRIX rightShoulderMod = XMMatrixAffineTransformation(rightS, idenQ, rightModQ, rightT);

			const XMMATRIX leftShoulderAdd = XMMatrixMultiply(leftShoulderMod, XMMatrixInverse(nullptr, leftShoulder));
			const XMMATRIX rightShoulderAdd = XMMatrixMultiply(rightShoulderMod, XMMatrixInverse(nullptr, rightShoulder));

			XMStoreFloat4x4(&additionalBind[leftShoulderRef].transform, leftShoulderAdd);
			XMStoreFloat4x4(&additionalBind[rightShoulderRef].transform, rightShoulderAdd);
			additionalBind[leftShoulderRef].isIKModified = true;
			additionalBind[rightShoulderRef].isIKModified = true;
			for (uint i = 1; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
			{
				const uint8 tarIndex = tarAvatar->jointReference[i];
				const uint8 srcIndex = srcAvatar->jointReference[i];

				if (tarIndex == JSkeletonFixedData::incorrectJointIndex ||
					srcIndex == JSkeletonFixedData::incorrectJointIndex)
					continue;

				const uint8 parentRef = JAvatar::jointReferenceParent[i];
				const uint8 tarParentIndex = tarAvatar->jointReference[parentRef];
				const uint8 srcParentIndex = srcAvatar->jointReference[parentRef];

				if (additionalBind[parentRef].isIKModified && !additionalBind[i].isIKModified)
				{
					const XMMATRIX srcParentAdditionalM = XMLoadFloat4x4(&additionalBind[parentRef].transform);
					const XMMATRIX srcParentWorldBind = srcSkeleton->GetBindPose(srcParentIndex);
					const XMMATRIX srcParentWorldBindMod = XMMatrixMultiply(srcParentAdditionalM, srcParentWorldBind);

					const XMMATRIX srcWorldBind = srcSkeleton->GetBindPose(srcIndex);
					const XMMATRIX srcLocalBind = XMMatrixMultiply(srcWorldBind, XMMatrixInverse(nullptr, srcParentWorldBind));
					const XMMATRIX srcWorldBindMod = XMMatrixMultiply(srcLocalBind, srcParentWorldBindMod);
					const XMMATRIX srcAdditionalM = XMMatrixMultiply(srcWorldBindMod, XMMatrixInverse(nullptr, srcWorldBind));
					XMStoreFloat4x4(&additionalBind[i].transform, srcAdditionalM);
					additionalBind[i].isIKModified = true;
				}
			}
			//StuffAdjustResult(updateData, srcSkeletonAsset, tarSkeletonAsset, additionalBind, bindPose);
		}
	}
}