#include"JAnimationFSMdiagram.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"
#include"../../GameTimer/JGameTimer.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
#include"../../../Window/Editor/Diagram/EditorDiagram.h"
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include<DirectXMath.h>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		JAnimationFSMdiagram::JAnimationFSMdiagram(const std::string& name, IJFSMconditionStorageUser* conditionStorage)
			:JFSMdiagram(name, conditionStorage)
		{
			editorDiagram = std::make_unique<EditorDiagram>();
		}
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
					size_t nextStateId = nextTransition->GetOutputId();
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
		std::vector<JAnimationFSMstate*>::const_iterator JAnimationFSMdiagram::GetAnimationFSMstateVectorHandle(_Out_ uint& stateCount)noexcept
		{
			stateCount = (uint)stateCash.size();
			return stateCash.cbegin();
		}
		EditorDiagram* JAnimationFSMdiagram::GetEditorDiagram()noexcept
		{
			return editorDiagram.get();
		}
		void JAnimationFSMdiagram::SetStateName(const std::string& oldName, const std::string& newName)
		{
			JFSMdiagram::SetStateName(oldName, newName);
		}
		void JAnimationFSMdiagram::SetAnimationClip(const std::string& stateName, JAnimationClip* clip)
		{
			JFSMstate* state = GetState(stateName);
			JAnimationFSMstate* aniFsm = static_cast<JAnimationFSMstate*>(state);
			if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
				static_cast<JAnimationFSMstateClip*>(state)->SetClip(clip);
		}
		void JAnimationFSMdiagram::SetTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& newConditionName, const uint oldConditionIndex)noexcept
		{
			JFSMdiagram::SetTransitionCondition(stateName, outputStateName, newConditionName, oldConditionIndex);
		}
		void JAnimationFSMdiagram::SetTransitionCondtionOnValue(const std::string& stateName, const std::string& outputStateName, const uint conditionIndex, const float value)noexcept
		{
			JFSMdiagram::SetTransitionCondtionOnValue(stateName, outputStateName, conditionIndex, value);
		}
		JAnimationFSMstate* JAnimationFSMdiagram::AddAnimationClipState()noexcept
		{
			std::unique_ptr<JAnimationFSMstateClip> stateClip = std::make_unique<JAnimationFSMstateClip>();
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
		JAnimationFSMtransition* JAnimationFSMdiagram::AddAnimationTransition(const std::string& stateName, const std::string& outputStateName)noexcept
		{
			size_t outputId;
			if (GetStateId(outputStateName, outputId))
			{
				std::unique_ptr<JAnimationFSMtransition> animationTransition = std::make_unique< JAnimationFSMtransition>(outputId);
				JFSMtransition* fsmCash = AddTransition(stateName, std::move(animationTransition));
				if (fsmCash != nullptr)
					return static_cast<JAnimationFSMtransition*>(fsmCash);
				else
					return nullptr;
			}
			else
				return nullptr;
		}
		bool JAnimationFSMdiagram::EraseAnimationState(const std::string& stateName)noexcept
		{
			JFSMstate* tarState = GetState(stateName);
			if (tarState != nullptr)
			{
				const size_t stateId = tarState->GetId();
				const uint cashCount = (uint)stateCash.size();
				for (uint i = 0; i < cashCount; ++i)
				{
					if (stateCash[i]->GetId() == stateId)
					{
						stateCash.erase(stateCash.begin() + i);
						break;
					}
				}
				for (uint i = 0; i < cashCount - 1; ++i)
					stateCash[i]->EraseTransition(tarState->GetId());

				JFSMdiagram::EraseState(stateId);
				return true;
			}
			else
				return false;
		}
		bool JAnimationFSMdiagram::EraseAnimationTransition(const std::string& stateName, const std::string& outputStateName)noexcept
		{
			return JFSMdiagram::EraseTransition(stateName, outputStateName);
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