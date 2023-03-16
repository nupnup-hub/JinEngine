#include"JAnimationFSMdiagram.h"
#include"JAnimationUpdateData.h"
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
		using JDiagramData = JAnimationUpdateData::DiagramData;
		void JAnimationFSMdiagram::Initialize(JAnimationUpdateData* updateData, const uint layerNumber)noexcept
		{
			JFSMdiagram::Initialize();
			JFSMstate* state = GetNowState();
			
			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			if (state != nullptr)
				diagramData.nowState = static_cast<JAnimationFSMstate*>(GetNowState());
			else
				diagramData.nowState = nullptr;
			diagramData.nextState = nullptr;
			diagramData.preTransition = nullptr;
			diagramData.nowTransition = nullptr;

			PreprocessSkeletonBindPose(updateData);
		}
		void JAnimationFSMdiagram::Enter(JAnimationUpdateData* updateData, const uint layerNumber)
		{
			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			if (diagramData.nowState != nullptr)
				diagramData.nowState->Enter(updateData, layerNumber, 0);
		}
		void JAnimationFSMdiagram::Update(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
		{ 
			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			if (diagramData.nextState == nullptr)
			{
				diagramData.nowTransition = diagramData.nowState->FindNextStateTransition(updateData, layerNumber, 0);
				if (diagramData.nowTransition != nullptr)
				{
					size_t nextStateId = diagramData.nowTransition->GetOutputStateGuid();
					diagramData.nextState = static_cast<JAnimationFSMstate*>(GetState(nextStateId));
					 
					diagramData.nextState->Enter(updateData, layerNumber, 1);
					diagramData.blender.Initialize(updateData->timer->TotalTime(), updateData->timer->TotalTime() + diagramData.nowTransition->GetDurationTime());
				}
			}

			if (diagramData.blender.IsActivated())
			{ 
				if (diagramData.blender.IsBlenderEnd(updateData->timer->TotalTime()))
				{
					diagramData.preTransition = diagramData.nowTransition;
					diagramData.nowState = diagramData.nextState;
					diagramData.nowTransition = nullptr;
					diagramData.nextState = nullptr;
					diagramData.animationTimes[0] = diagramData.animationTimes[1];
					diagramData.blender.Clear();
				}
			}

			if (diagramData.nextState != nullptr)
			{
				diagramData.nowState->Update(updateData, layerNumber, 0);
				diagramData.nextState->Update(updateData, layerNumber, 1);
				CrossFading(updateData, animationConstatns, layerNumber);
			}
			else
			{
				diagramData.nowState->Update(updateData, layerNumber, 0);
				StuffFinalTransform(updateData, animationConstatns, layerNumber);
			}

			const bool canLoop = diagramData.animationTimes[0].IsOverEndTime() && diagramData.nowState->CanLoop();
			if (canLoop)
				diagramData.nowState->Enter(updateData, layerNumber, 0);
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
		JAnimationFSMtransition* JAnimationFSMdiagram::GetTransition(const size_t transitionGuid)noexcept
		{
			const std::vector<JFSMstate*>& stateVec = GetStateVec();
			for (const auto& data : stateVec)
			{
				JFSMtransition* transition = data->GetTransition(transitionGuid);
				if (transition != nullptr)
					return static_cast<JAnimationFSMtransition*>(transition);
			}
			return nullptr;
		}
		const std::vector<JFSMstate*>& JAnimationFSMdiagram::GetStateVec()noexcept
		{
			return JFSMdiagram::GetStateVec();
		}
		void JAnimationFSMdiagram::SetClip(const size_t stateGuid, Core::JUserPtr<JAnimationClip> clip)noexcept
		{
			JFSMstate* state = GetState(stateGuid);
			JAnimationFSMstate* aniFsm = static_cast<JAnimationFSMstate*>(state);
			if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
				static_cast<JAnimationFSMstateClip*>(state)->SetClip(clip);
		}
		void JAnimationFSMdiagram::StuffFinalTransform(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
		{
			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			uint size = (uint)updateData->modelSkeleton->GetSkeleton()->GetJointCount();
			JSkeleton* skeleton = updateData->modelSkeleton->GetSkeleton();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();
			XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			for (uint i = 0; i < size; ++i)
				XMStoreFloat4x4(&animationConstatns.boneTransforms[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i),
					XMLoadFloat4x4(&diagramData.worldTransform[0][i]))));
		}
		void JAnimationFSMdiagram::CrossFading(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
		{
			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			float rate = diagramData.blender.GetBlnederValue(updateData->timer->TotalTime());
			if (rate > 1.0f)
				MessageBox(0, L"Over", std::to_wstring(rate).c_str(), 0);
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

			JSkeleton* skeleton = updateData->modelSkeleton->GetSkeleton();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();
			uint size = (uint)updateData->modelSkeleton->GetSkeleton()->GetJointCount();

			for (uint i = 0; i < size; ++i)
			{
				XMMATRIX finalStM = XMLoadFloat4x4(&diagramData.worldTransform[0][i]);
				XMMATRIX finalEdM = XMLoadFloat4x4(&diagramData.worldTransform[1][i]);

				XMMatrixDecompose(&stS, &stQ, &stT, finalStM);
				XMMatrixDecompose(&edS, &edQ, &edT, finalEdM);

				finalS = XMVectorLerp(stS, edS, rate);
				finalQ = XMQuaternionSlerp(stQ, edQ, rate);
				finalT = XMVectorLerp(stT, edT, rate);

				XMStoreFloat4x4(&animationConstatns.boneTransforms[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), XMMatrixAffineTransformation(finalS, zero, finalQ, finalT))));
				//XMStoreFloat4x4(&finalTransform[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), skeleton->GetBindPose(i))));
			}
		}
		void JAnimationFSMdiagram::PreprocessSkeletonBindPose(JAnimationUpdateData* updateData)noexcept
		{
			std::vector<JSkeletonAsset*> skeletonVec;
			uint stateSize = GetStateCount();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();

			if (updateData->skeletonBlendRate[0].find(srcGuid) == updateData->skeletonBlendRate[0].end())
				updateData->skeletonBlendRate[0].emplace(srcGuid, 0.0f);
			if (updateData->skeletonBlendRate[1].find(srcGuid) == updateData->skeletonBlendRate[0].end())
				updateData->skeletonBlendRate[1].emplace(srcGuid, 0.0f);

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
					if (tarGuid != srcGuid && updateData->additionalBind.find(tarGuid) == updateData->additionalBind.end())
					{
						std::vector<JAnimationAdditionalBind> additionalBind(JSkeletonFixedData::maxAvatarJointCount);
						additionalBind.reserve(JSkeletonFixedData::maxAvatarJointCount);
						JAnimationRetargeting::CalculateAdditionalBindPose(updateData, updateData->modelSkeleton, skeletonVec[i], additionalBind);
						updateData->additionalBind.emplace(skeletonVec[i]->GetGuid(), additionalBind);
						updateData->skeletonBlendRate->emplace(skeletonVec[i]->GetGuid(), 0.0f);
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
				JFileIOHelper::StoreFsmObjectIden(stream, fsmState);
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
				newDiagram->GetStateByIndex(i)->StreamInterface()->LoadData(stream);
			
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