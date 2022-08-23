#include"JAnimationFSMdiagram.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"
#include"../../GameTimer/JGameTimer.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include<DirectXMath.h>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		JAnimationFSMdiagram::JAnimationFSMdiagram(const std::string& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage)
			:JFSMdiagram(name, guid, conditionStorage)
		{ }
		JAnimationFSMdiagram::~JAnimationFSMdiagram() {}
		void JAnimationFSMdiagram::Initialize(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept
		{
			JFSMdiagram::Initialize();
			JFSMstate* state = GetNowState();
			if (state != nullptr)
				nowState = static_cast<JAnimationFSMstate*>(GetNowState());
			else
				nowState = nullptr;
			nextState = nullptr;
			nextTransition = nullptr;

			PreprocessSkeletonBindPose(animationShareData, srcSkeletonAsset);
		}
		void JAnimationFSMdiagram::Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)
		{
			if (nowState != nullptr)
				nowState->Enter(animationTime, animationShareData, srcSkeletonAsset, 0);
		}
		void JAnimationFSMdiagram::Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
		{
			if (nextState == nullptr)
			{
				nextTransition = nowState->FindNextStateTransition(animationTime);
				if (nextTransition != nullptr)
				{
					size_t nextStateId = nextTransition->GetOutputStateGuId();
					nextState = static_cast<JAnimationFSMstate*>(GetState(nextStateId));
					nextState->Enter(animationTime, animationShareData, srcSkeletonAsset, nextTransition->GetTargetStateOffset());
					blender.Initialize(JGameTimer::Instance().TotalTime(), JGameTimer::Instance().TotalTime() + nextTransition->GetDurationTime());
				}
			}

			if (nextState != nullptr)
			{
				if (blender.IsBlenderEnd(JGameTimer::Instance().TotalTime()))
				{
					nowState = nextState;
					nextState = nullptr;
					nextTransition = nullptr;
					nowState->Update(animationTime, animationShareData, srcSkeletonAsset, 0);
					StuffFinalTransform(animationShareData, srcSkeletonAsset, animationConstatns);
				}
				else
				{
					nowState->Update(animationTime, animationShareData, srcSkeletonAsset, 0);
					nextState->Update(animationTime, animationShareData, srcSkeletonAsset, 1);
					CrossFading(animationShareData, srcSkeletonAsset, animationConstatns);
				}
			}
			else
			{
				nowState->Update(animationTime, animationShareData, srcSkeletonAsset, 0);
				StuffFinalTransform(animationShareData, srcSkeletonAsset, animationConstatns);
			}
		}
		bool JAnimationFSMdiagram::HasAnimationData()noexcept
		{
			return nowState != nullptr;
		}
		JAnimationFSMstate* JAnimationFSMdiagram::GetState(const size_t stateGuid)noexcept
		{
			return GetState(stateGuid);
		}
		std::vector<JAnimationFSMstate*>& JAnimationFSMdiagram::GetStateVec()noexcept
		{
			return stateCash;
		}
		void JAnimationFSMdiagram::SetStateName(const size_t stateGuid, const std::string& newName)noexcept
		{
			JFSMdiagram::SetStateName(stateGuid, newName);
		}
		void JAnimationFSMdiagram::SetAnimationClip(const size_t stateGuid, JAnimationClip* clip)noexcept
		{
			JFSMstate* state = GetState(stateGuid);
			JAnimationFSMstate* aniFsm = static_cast<JAnimationFSMstate*>(state);
			if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
				static_cast<JAnimationFSMstateClip*>(state)->SetClip(clip);
		}
		void JAnimationFSMdiagram::SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept
		{
			JFSMdiagram::SetTransitionCondition(inputStateGuid, outputStateGuid, conditionGuid, conditionIndex);
		}
		void JAnimationFSMdiagram::SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept
		{
			JFSMdiagram::SetTransitionCondtionOnValue(inputStateGuid, outputStateGuid, conditionIndex, value);
		}
		JAnimationFSMstate* JAnimationFSMdiagram::CreateAnimationClipState(const std::string& name, const size_t guid)noexcept
		{
			std::unique_ptr<JAnimationFSMstateClip> stateClip = std::make_unique<JAnimationFSMstateClip>(name, guid);
			JFSMstate* fsmCash = AddState(std::move(stateClip));
			if (fsmCash != nullptr)
			{
				JAnimationFSMstate* aniFsmCash = static_cast<JAnimationFSMstate*>(fsmCash);
				stateCash.push_back(aniFsmCash);
				return aniFsmCash;
			}
			else
				return  nullptr;
		}
		JAnimationFSMtransition* JAnimationFSMdiagram::CreateAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept
		{
			std::unique_ptr<JAnimationFSMtransition> animationTransition = std::make_unique< JAnimationFSMtransition>(outputStateGuid);
			JFSMtransition* fsmCash = AddTransition(inputStateGuid, std::move(animationTransition));
			if (fsmCash != nullptr)
				return static_cast<JAnimationFSMtransition*>(fsmCash);
			else
				return nullptr;
		}
		bool JAnimationFSMdiagram::DestroyAnimationState(const size_t stateGuid)noexcept
		{
			JFSMstate* tarState = GetState(stateGuid);
			if (tarState != nullptr)
			{
				const size_t stateId = tarState->GetGuid();
				const uint cashCount = (uint)stateCash.size();
				for (uint i = 0; i < cashCount; ++i)
				{
					if (stateCash[i]->GetGuid() == stateId)
					{
						stateCash.erase(stateCash.begin() + i);
						break;
					}
				}
				for (uint i = 0; i < cashCount - 1; ++i)
					stateCash[i]->RemoveTransition(tarState->GetGuid());

				JFSMdiagram::DestroyState(stateId);
				return true;
			}
			else
				return false;
		}
		bool JAnimationFSMdiagram::DestroyAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept
		{
			return JFSMdiagram::RemoveTransition(inputStateGuid, outputStateGuid);
		}
		void JAnimationFSMdiagram::Clear()noexcept
		{
			JFSMdiagram::Clear();
		}
		bool JAnimationFSMdiagram::HasState()noexcept
		{
			return nowState != nullptr;
		}
		void JAnimationFSMdiagram::StuffFinalTransform(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept
		{
			uint size = (uint)srcSkeletonAsset->GetSkeleton()->GetJointCount();
			JSkeleton* skeleton = srcSkeletonAsset->GetSkeleton();
			size_t srcGuid = srcSkeletonAsset->GetGuid();
			XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			for (uint i = 0; i < size; ++i)
				XMStoreFloat4x4(&animationConstatns.boneTransforms[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i),
					XMLoadFloat4x4(&animationShareData.localTransform[0][i]))));
		}
		void JAnimationFSMdiagram::CrossFading(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept
		{
			float rate = blender.GetBlnederValue(JGameTimer::Instance().TotalTime());
			XMVECTOR stS;
			XMVECTOR stQ;
			XMVECTOR stT;
			XMVECTOR edS;
			XMVECTOR edQ;
			XMVECTOR edT;

			XMVECTOR finalS;
			XMVECTOR finalQ;
			XMVECTOR finalT;

			XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			JSkeleton* skeleton = srcSkeletonAsset->GetSkeleton();
			size_t srcGuid = srcSkeletonAsset->GetGuid();
			uint size = (uint)srcSkeletonAsset->GetSkeleton()->GetJointCount();

			for (uint i = 0; i < size; ++i)
			{
				XMMATRIX finalStM = XMLoadFloat4x4(&animationShareData.localTransform[0][i]);
				XMMATRIX finalEdM = XMLoadFloat4x4(&animationShareData.localTransform[1][i]);

				XMMatrixDecompose(&stS, &stQ, &stT, finalStM);
				XMMatrixDecompose(&edS, &edQ, &edT, finalEdM);

				finalS = XMVectorLerp(stS, edS, rate);
				finalQ = XMQuaternionSlerp(stQ, edQ, rate);
				finalT = XMVectorLerp(stT, edT, rate);

				XMStoreFloat4x4(&animationConstatns.boneTransforms[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), XMMatrixAffineTransformation(finalS, zero, finalQ, finalT))));
				//XMStoreFloat4x4(&finalTransform[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), skeleton->GetBindPose(i))));
			}
		}
		void JAnimationFSMdiagram::PreprocessSkeletonBindPose(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept
		{
			std::vector<JSkeletonAsset*> skeletonVec;
			uint stateSize = GetStateCount();
			size_t srcGuid = srcSkeletonAsset->GetGuid();

			if (animationShareData.skeletonBlendRate[0].find(srcGuid) == animationShareData.skeletonBlendRate[0].end())
				animationShareData.skeletonBlendRate[0].emplace(srcGuid, 0.0f);
			if (animationShareData.skeletonBlendRate[1].find(srcGuid) == animationShareData.skeletonBlendRate[0].end())
				animationShareData.skeletonBlendRate[1].emplace(srcGuid, 0.0f);

			for (uint i = 0; i < stateSize; ++i)
			{
				JAnimationFSMstate* state = static_cast<JAnimationFSMstate*>(GetStateByIndex(i));
				state->GetRegisteredSkeleton(skeletonVec);
			}

			uint skeletonVecSize = (uint)skeletonVec.size();

			for (uint i = 0; i < skeletonVecSize; ++i)
			{
				if (skeletonVec[i] != nullptr)
				{
					size_t tarGuid = skeletonVec[i]->GetGuid();
					if (tarGuid != srcGuid && animationShareData.additionalBind.find(tarGuid) == animationShareData.additionalBind.end())
					{
						std::vector<JAnimationAdditionalBind> additionalBind(JSkeletonFixedData::maxAvatarJointCount);
						additionalBind.reserve(JSkeletonFixedData::maxAvatarJointCount);
						JAnimationRetargeting::CalculateAdditionalBindPose(animationShareData, srcSkeletonAsset, skeletonVec[i], additionalBind);
						animationShareData.additionalBind.emplace(skeletonVec[i]->GetGuid(), additionalBind);
						animationShareData.skeletonBlendRate->emplace(skeletonVec[i]->GetGuid(), 0.0f);
					}
				}
			}
		}
	}
}