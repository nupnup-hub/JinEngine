#include"JAnimationFSMdiagram.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h" 
#include"../JFSMfactory.h" 
#include"../../File/JFileIOHelper.h"
#include"../../Time/JGameTimer.h"
#include"../../Reflection/JTypeTemplate.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../Utility/JCommonUtility.h"
#include<DirectXMath.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
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
					size_t nextStateId = nextTransition->GetOutputStateGuid();
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
		bool JAnimationFSMdiagram::HasNowState()const noexcept
		{
			return nowState != nullptr;
		}
		bool JAnimationFSMdiagram::CanCreateState()const noexcept
		{
			return GetStateCount() < maxNumberOffState;
		}
		JAnimationFSMstate* JAnimationFSMdiagram::GetState(const size_t stateGuid)noexcept
		{
			JFSMstate* state = JFSMdiagram::GetState(stateGuid);
			if (state != nullptr)
				return static_cast<JAnimationFSMstate*>(state);
			else
				return nullptr; 
		}
		JAnimationFSMstate* JAnimationFSMdiagram::GetStateByIndex(const uint index)noexcept
		{
			return static_cast<JAnimationFSMstate*>(JFSMdiagram::GetStateByIndex(index));
		}
		const std::vector<JFSMstate*>& JAnimationFSMdiagram::GetStateVec()noexcept
		{
			return JFSMdiagram::GetStateVec();
		}
		void JAnimationFSMdiagram::SetClip(const size_t stateGuid, JAnimationClip* clip)noexcept
		{
			JFSMstate* state = GetState(stateGuid);
			JAnimationFSMstate* aniFsm = static_cast<JAnimationFSMstate*>(state);
			if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
				static_cast<JAnimationFSMstateClip*>(state)->SetClip(clip);
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
		J_FILE_IO_RESULT JAnimationFSMdiagram::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
			JFileIOHelper::StoreFsmObjectIden(stream, this);

			const std::vector<JFSMstate*>& stateVec = GetStateVec();
			const uint stateCount = (uint)stateVec.size();
			JFileIOHelper::StoreAtomicData(stream, L"StateCount:", stateCount);
		 
			for (uint i = 0; i < stateCount; ++i)
			{
				JAnimationFSMstate* fsmState = GetStateByIndex(i);
				JFileIOHelper::StoreHasObjectIden(stream, fsmState);
				JFileIOHelper::StoreEnumData(stream, L"StateType:", fsmState->GetStateType());
			}
			for (uint i = 0; i < stateCount; ++i)
				GetStateByIndex(i)->StreamInterface()->StoreData(stream);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		JAnimationFSMdiagram* JAnimationFSMdiagram::LoadData(std::wifstream& stream, JFSMdiagramOwnerInterface* fsmOwner)
		{
			if (!stream.is_open())
				return nullptr;
			 
			std::wstring name;
			size_t guid;
			J_FSM_OBJECT_TYPE type;

			JFileIOHelper::LoadFsmObjectIden(stream, name, guid, type);
  
			JAnimationFSMdiagram* newDiagram = JFFI<JAnimationFSMdiagram>::Create(JPtrUtil::MakeOwnerPtr<InitData>(name, guid, fsmOwner));
			JUserPtr<JAnimationFSMdiagram> diagramUser = Core::GetUserPtr(newDiagram);
			uint stateCount = 0;
			JFileIOHelper::LoadAtomicData(stream, stateCount);

			for (uint i = 0; i < stateCount; ++i)
			{
				J_ANIMATION_STATE_TYPE stateType;
				JAnimationFSMstate* newState = nullptr;
				JFileIOHelper::LoadFsmObjectIden(stream, name, guid, type);
				JFileIOHelper::LoadEnumData(stream, stateType);

				if (stateType == J_ANIMATION_STATE_TYPE::CLIP)
					newState = JFFI<JAnimationFSMstateClip>::Create(JPtrUtil::MakeOwnerPtr<JFSMstate::InitData>(name, guid, diagramUser));
				else if (stateType == J_ANIMATION_STATE_TYPE::BLEND_TREE)
					;//¹Ì±¸Çö 
			} 
			for (uint i = 0; i < stateCount; ++i)
				newDiagram->GetStateByIndex(i)->StreamInterface()->LoadData(stream, *newDiagram->GetStroageUser());
			
			return newDiagram;
		}
		void JAnimationFSMdiagram::RegisterJFunc()
		{ 
			auto createDiagramLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::DIAGRAM)
				{
					JFSMdiagramInitData* digramInitData = static_cast<JFSMdiagramInitData*>(initData.Get());
					JOwnerPtr<JAnimationFSMdiagram> ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationFSMdiagram>(*digramInitData);
					JAnimationFSMdiagram* newDiagram = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newDiagram;
				}
				return nullptr;
			};
			JFFI<JAnimationFSMdiagram>::Register(createDiagramLam);
		}
		JAnimationFSMdiagram::JAnimationFSMdiagram(const JFSMdiagramInitData& initData)
			:JFSMdiagram(initData)
		{ }
		JAnimationFSMdiagram::~JAnimationFSMdiagram() {}
	}
}