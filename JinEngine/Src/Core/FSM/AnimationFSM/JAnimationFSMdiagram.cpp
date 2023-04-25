#include"JAnimationFSMdiagram.h"
#include"JAnimationFSMdiagramPrivate.h"
#include"JAnimationUpdateData.h"
#include"JAnimationFSMstatePrivate.h"
#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h" 
#include"JBlender.h" 
#include"JAnimationPostProcessing.h"
#include"JAnimationRetargeting.h" 

#include"../../File/JFileIOHelper.h"
#include"../../Time/JGameTimer.h"
#include"../../Reflection/JTypeTemplate.h"
#include"../../Identity/JIdenCreator.h"
#include"../../Identity/JIdentifierImplBase.h"
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
		namespace
		{
			using JDiagramData = JAnimationUpdateData::DiagramData;
			using _StateIOInterface = JAnimationFSMstatePrivate::AssetDataIOInterface;
			using _StateUpdateInterface = JAnimationFSMstatePrivate::UpdateInterface;
		}
		namespace
		{
			static JAnimationFSMdiagramPrivate aPrivate;

			_StateIOInterface* StateIOInterface(JAnimationFSMstate* state)
			{
				return &static_cast<_StateIOInterface&>(static_cast<JAnimationFSMstatePrivate&>(state->GetPrivateInterface()).GetAssetDataIOInterface());
			}
			_StateUpdateInterface* StateUpdateInterface(JAnimationFSMstate* state)
			{
				return &static_cast<_StateUpdateInterface&>(static_cast<JAnimationFSMstatePrivate&>(state->GetPrivateInterface()).GetUpdateInterface());
			}
		}
		 
		class JAnimationFSMdiagram::JAnimationFSMdiagramImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMdiagramImpl)
		public:
			JAnimationFSMdiagram* thisDiagram = nullptr;
		public:
			JAnimationFSMdiagramImpl(const InitData& initData, JAnimationFSMdiagram* thisDiagram)
				:thisDiagram(thisDiagram)
			{}
			~JAnimationFSMdiagramImpl()
			{}
		public:
			void Initialize(JAnimationUpdateData* updateData, const uint layerNumber)noexcept
			{
				thisDiagram->Initialize();
				JFSMstate* state = thisDiagram->GetNowState();

				JDiagramData& diagramData = updateData->diagramData[layerNumber];
				if (state != nullptr)
					diagramData.nowState = static_cast<JAnimationFSMstate*>(thisDiagram->GetNowState());
				else
					diagramData.nowState = nullptr;
				diagramData.nextState = nullptr;
				diagramData.preTransition = nullptr;
				diagramData.nowTransition = nullptr;

				PreprocessSkeletonBindPose(updateData);
			}
			void Enter(JAnimationUpdateData* updateData, const uint layerNumber)
			{
				JDiagramData& diagramData = updateData->diagramData[layerNumber];
				if (diagramData.nowState != nullptr)
					StateUpdateInterface(diagramData.nowState)->Enter(diagramData.nowState, updateData, layerNumber, 0);
			}
			void Update(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
			{
				JDiagramData& diagramData = updateData->diagramData[layerNumber];
				if (diagramData.nextState == nullptr)
				{
					diagramData.nowTransition = diagramData.nowState->FindNextStateTransition(updateData, layerNumber, 0);
					if (diagramData.nowTransition != nullptr)
					{
						size_t nextStateId = diagramData.nowTransition->GetOutputStateGuid();
						diagramData.nextState = static_cast<JAnimationFSMstate*>(thisDiagram->GetState(nextStateId));

						StateUpdateInterface(diagramData.nextState)->Enter(diagramData.nextState, updateData, layerNumber, 1);
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
					StateUpdateInterface(diagramData.nowState)->Update(diagramData.nowState, updateData, layerNumber, 0);
					StateUpdateInterface(diagramData.nextState)->Update(diagramData.nextState, updateData, layerNumber, 1);
					CrossFading(updateData, animationConstatns, layerNumber);
				}
				else
				{
					StateUpdateInterface(diagramData.nowState)->Update(diagramData.nowState, updateData, layerNumber, 0);
					StuffFinalTransform(updateData, animationConstatns, layerNumber);
				}

				const bool canLoop = diagramData.animationTimes[0].IsOverEndTime() && diagramData.nowState->CanLoop();
				if (canLoop)
					StateUpdateInterface(diagramData.nowState)->Enter(diagramData.nowState, updateData, layerNumber, 0);
			}
		public:
			void StuffFinalTransform(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
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
			void CrossFading(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
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
			void PreprocessSkeletonBindPose(JAnimationUpdateData* updateData)noexcept
			{
				std::vector<JSkeletonAsset*> skeletonVec;
				uint stateSize = thisDiagram->GetStateCount();
				size_t srcGuid = updateData->modelSkeleton->GetGuid();

				if (updateData->skeletonBlendRate[0].find(srcGuid) == updateData->skeletonBlendRate[0].end())
					updateData->skeletonBlendRate[0].emplace(srcGuid, 0.0f);
				if (updateData->skeletonBlendRate[1].find(srcGuid) == updateData->skeletonBlendRate[0].end())
					updateData->skeletonBlendRate[1].emplace(srcGuid, 0.0f);

				for (uint i = 0; i < stateSize; ++i)
				{
					JAnimationFSMstate* state = static_cast<JAnimationFSMstate*>(thisDiagram->GetStateByIndex(i));
					StateUpdateInterface(state)->GetRegisteredSkeleton(state, skeletonVec); 
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
		public:
			//state가 소유하는  transtion은 has state object이므로
			//state먼저 모두 생성한 후 state detail을 load 한다
			static JAnimationFSMdiagram* LoadAssetData(std::wifstream& stream, JFSMdiagramOwnerInterface* fsmOwner)
			{
				if (!stream.is_open())
					return nullptr;

				std::wstring name;
				size_t guid;
				J_FSM_OBJECT_TYPE type;

				JFileIOHelper::LoadFsmObjectIden(stream, name, guid, type);

				JAnimationFSMdiagram* newDiagram = JICI::Create<JAnimationFSMdiagram>(name, guid, fsmOwner);
				JUserPtr<JAnimationFSMdiagram> diagramUser = GetUserPtr(newDiagram);
				uint stateCount = 0;
				JFileIOHelper::LoadAtomicData(stream, stateCount);

				for (uint i = 0; i < stateCount; ++i)
				{
					J_ANIMATION_STATE_TYPE stateType;
					JAnimationFSMstate* newState = nullptr;
					JFileIOHelper::LoadFsmObjectIden(stream, name, guid, type);
					JFileIOHelper::LoadEnumData(stream, stateType);

					if (stateType == J_ANIMATION_STATE_TYPE::CLIP)
						newState = JICI::Create<JAnimationFSMstateClip>(name, guid, diagramUser);
					else if (stateType == J_ANIMATION_STATE_TYPE::BLEND_TREE)
						;//미구현 
				}
				for (uint i = 0; i < stateCount; ++i)
				{
					JAnimationFSMstate* state = newDiagram->GetStateByIndex(i);
					StateIOInterface(state)->LoadAssetData(stream, state);
				}
				return newDiagram;
			}
			static J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, JAnimationFSMdiagram* diagram)
			{
				if (!stream.is_open())
					return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				JFileIOHelper::StoreFsmObjectIden(stream, diagram);

				const std::vector<JFSMstate*>& stateVec = diagram->GetStateVec();
				const uint stateCount = (uint)stateVec.size();
				JFileIOHelper::StoreAtomicData(stream, L"StateCount:", stateCount);

				for (uint i = 0; i < stateCount; ++i)
				{
					JAnimationFSMstate* fsmState = diagram->GetStateByIndex(i);
					JFileIOHelper::StoreFsmObjectIden(stream, fsmState);
					JFileIOHelper::StoreEnumData(stream, L"StateType:", fsmState->GetStateType());
				}
				for (uint i = 0; i < stateCount; ++i)
				{
					JAnimationFSMstate* state = diagram->GetStateByIndex(i);
					StateIOInterface(state)->StoreAssetData(stream, state);
				}
				return J_FILE_IO_RESULT::SUCCESS;
			}
		public:
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JAnimationFSMdiagram::StaticTypeInfo(), aPrivate);
			}
		};

		JAnimationFSMdiagram::InitData::InitData(JFSMdiagramOwnerInterface* ownerInterface)
			:JFSMdiagram::InitData(JAnimationFSMdiagram::StaticTypeInfo(), ownerInterface)
		{}
		JAnimationFSMdiagram::InitData::InitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface)
			: JFSMdiagram::InitData(JAnimationFSMdiagram::StaticTypeInfo(), name, guid, ownerInterface)
		{}
	  
		Core::JIdentifierPrivate& JAnimationFSMdiagram::GetPrivateInterface()const noexcept
		{
			return aPrivate;
		}
		JAnimationFSMstate* JAnimationFSMdiagram::GetState(const size_t stateGuid)noexcept
		{
			return static_cast<JAnimationFSMstate*>(JFSMdiagram::GetState(stateGuid));
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
		void JAnimationFSMdiagram::SetClip(const size_t stateGuid, JUserPtr<JAnimationClip> clip)noexcept
		{
			JFSMstate* state = GetState(stateGuid);
			JAnimationFSMstate* aniFsm = static_cast<JAnimationFSMstate*>(state);
			if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
				static_cast<JAnimationFSMstateClip*>(state)->SetClip(clip);
		}
		JAnimationFSMdiagram::JAnimationFSMdiagram(const InitData& initData)
			:JFSMdiagram(initData), impl(std::make_unique<JAnimationFSMdiagramImpl>(initData, this))
		{ }
		JAnimationFSMdiagram::~JAnimationFSMdiagram() 
		{
			impl.reset();
		}

		using CreateInstanceInterface = JAnimationFSMdiagramPrivate::CreateInstanceInterface;
		using AssetDataIOInterface = JAnimationFSMdiagramPrivate::AssetDataIOInterface;
		using UpdateInterface = JAnimationFSMdiagramPrivate::UpdateInterface;

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<JDITypeDataBase>&& initData)
		{
			return JPtrUtil::MakeOwnerPtr<JAnimationFSMdiagram>(*static_cast<JAnimationFSMdiagram::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMdiagram::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		JAnimationFSMdiagram* AssetDataIOInterface::LoadAssetData(std::wifstream& stream, JFSMdiagramOwnerInterface* fsmOwner)
		{
			return JAnimationFSMdiagram::JAnimationFSMdiagramImpl::LoadAssetData(stream, fsmOwner);
		}
		J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, JAnimationFSMdiagram* diagram)
		{
			return JAnimationFSMdiagram::JAnimationFSMdiagramImpl::StoreAssetData(stream, diagram);
		}

		void UpdateInterface::Initialize(JAnimationFSMdiagram* diagram, JAnimationUpdateData* updateData, const uint layerNumber)noexcept
		{
			diagram->impl->Initialize(updateData, layerNumber);
		}
		void UpdateInterface::Enter(JAnimationFSMdiagram* diagram, JAnimationUpdateData* updateData, const uint layerNumber)
		{
			diagram->impl->Enter(updateData, layerNumber);
		}
		void UpdateInterface::Update(JAnimationFSMdiagram* diagram, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
		{
			diagram->impl->Update(updateData, animationConstatns, layerNumber);
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationFSMdiagramPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
	}
}