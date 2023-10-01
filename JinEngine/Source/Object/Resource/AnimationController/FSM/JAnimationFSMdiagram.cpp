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

#include"../../Skeleton/JSkeleton.h"
#include"../../Skeleton/JSkeletonAsset.h"
#include"../../Skeleton/JSkeletonFixedData.h"
#include"../../../JObjectFileIOHelper.h"
#include"../../../../Core/Time/JGameTimer.h"
#include"../../../../Core/Reflection/JTypeTemplate.h"
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Reflection/JTypeImplBase.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Graphic/Frameresource/JAnimationConstants.h"
#include<DirectXMath.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;

	namespace
	{
		using JDiagramData = JAnimationUpdateData::DiagramData;
		using _StateIOInterface = JAnimationFSMstatePrivate::AssetDataIOInterface;
		using _StateUpdateInterface = JAnimationFSMstatePrivate::UpdateInterface;
	}
	namespace
	{
		static JAnimationFSMdiagramPrivate aPrivate;

		_StateIOInterface* StateIOInterface(const JUserPtr<JAnimationFSMstate>& state)
		{
			return &static_cast<_StateIOInterface&>(static_cast<JAnimationFSMstatePrivate&>(state->PrivateInterface()).GetAssetDataIOInterface());
		}
		_StateUpdateInterface* StateUpdateInterface(const JUserPtr<JAnimationFSMstate>& state)
		{
			return &static_cast<_StateUpdateInterface&>(static_cast<JAnimationFSMstatePrivate&>(state->PrivateInterface()).GetUpdateInterface());
		}
	}

	class JAnimationFSMdiagram::JAnimationFSMdiagramImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMdiagramImpl)
	public:
		JWeakPtr<JAnimationFSMdiagram> thisPointer = nullptr;
	public:
		JAnimationFSMdiagramImpl(const InitData& initData, JAnimationFSMdiagram* thisDiagramRaw)
		{}
		~JAnimationFSMdiagramImpl()
		{}
	public:
		void Initialize(JAnimationUpdateData* updateData, const uint layerNumber)noexcept
		{
			thisPointer->Initialize();
			JUserPtr<Core::JFSMstate> state = thisPointer->GetNowState();

			JDiagramData& diagramData = updateData->diagramData[layerNumber];
			if (state != nullptr)
				diagramData.nowState = Core::ConvertChildUserPtr<JAnimationFSMstate>(thisPointer->GetNowState());
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
					diagramData.nextState = thisPointer->GetState(nextStateId);

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
			JUserPtr<JSkeleton> skeleton = updateData->modelSkeleton->GetSkeleton();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();
			XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			for (uint i = 0; i < size; ++i)
			{
				animationConstatns.boneTransforms[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i),
					diagramData.worldTransform[0][i].LoadXM())));
			}
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

			JUserPtr<JSkeleton> skeleton = updateData->modelSkeleton->GetSkeleton();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();
			uint size = (uint)updateData->modelSkeleton->GetSkeleton()->GetJointCount();

			for (uint i = 0; i < size; ++i)
			{
				XMMATRIX finalStM = diagramData.worldTransform[0][i].LoadXM();
				XMMATRIX finalEdM = diagramData.worldTransform[1][i].LoadXM();

				XMMatrixDecompose(&stS, &stQ, &stT, finalStM);
				XMMatrixDecompose(&edS, &edQ, &edT, finalEdM);

				finalS = XMVectorLerp(stS, edS, rate);
				finalQ = XMQuaternionSlerp(stQ, edQ, rate);
				finalT = XMVectorLerp(stT, edT, rate);

				animationConstatns.boneTransforms[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), XMMatrixAffineTransformation(finalS, zero, finalQ, finalT))));
				//XMStoreFloat4x4(&finalTransform[i], XMMatrixTranspose(XMMatrixMultiply(skeleton->GetInBindPose(i), skeleton->GetBindPose(i))));
			}
		}
		void PreprocessSkeletonBindPose(JAnimationUpdateData* updateData)noexcept
		{
			std::vector<JUserPtr<JSkeletonAsset>> skeletonVec;
			uint stateSize = thisPointer->GetStateCount();
			size_t srcGuid = updateData->modelSkeleton->GetGuid();

			if (updateData->skeletonBlendRate[0].find(srcGuid) == updateData->skeletonBlendRate[0].end())
				updateData->skeletonBlendRate[0].emplace(srcGuid, 0.0f);
			if (updateData->skeletonBlendRate[1].find(srcGuid) == updateData->skeletonBlendRate[0].end())
				updateData->skeletonBlendRate[1].emplace(srcGuid, 0.0f);

			for (uint i = 0; i < stateSize; ++i)
			{
				JUserPtr<JAnimationFSMstate> state = thisPointer->GetStateByIndex(i);
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
						JAnimationRetargeting::CalculateAdditionalBindPose(updateData, updateData->modelSkeleton.Get(), skeletonVec[i].Get(), additionalBind);
						updateData->additionalBind.emplace(skeletonVec[i]->GetGuid(), additionalBind);
						updateData->skeletonBlendRate->emplace(skeletonVec[i]->GetGuid(), 0.0f);
					}
				}
			}
		}
	public:
		//state가 소유하는  transtion은 has state object이므로
		//state먼저 모두 생성한 후 state detail을 load 한다
		static JUserPtr<JAnimationFSMdiagram> LoadAssetData(std::wifstream& stream, Core::JFSMdiagramOwnerInterface* fsmOwner)
		{
			if (!stream.is_open())
				return nullptr;

			std::wstring name;
			size_t guid;
			Core::J_FSM_OBJECT_TYPE type;

			JObjectFileIOHelper::LoadFsmIden(stream, name, guid, type);

			JUserPtr<JAnimationFSMdiagram> diagramUser = JICI::Create<JAnimationFSMdiagram>(name, guid, fsmOwner);
			uint stateCount = 0;
			JObjectFileIOHelper::LoadAtomicData(stream, stateCount);

			for (uint i = 0; i < stateCount; ++i)
			{
				J_ANIMATION_STATE_TYPE stateType;
				JUserPtr<JAnimationFSMstate> newState = nullptr;
				JObjectFileIOHelper::LoadFsmIden(stream, name, guid, type);
				JObjectFileIOHelper::LoadEnumData(stream, stateType);

				if (stateType == J_ANIMATION_STATE_TYPE::CLIP)
					newState = JICI::Create<JAnimationFSMstateClip>(name, guid, diagramUser);
				else if (stateType == J_ANIMATION_STATE_TYPE::BLEND_TREE)
					;//미구현 
			}
			for (uint i = 0; i < stateCount; ++i)
			{
				JUserPtr<JAnimationFSMstate> state = diagramUser->GetStateByIndex(i);
				StateIOInterface(state)->LoadAssetData(stream, state);
			}
			return diagramUser;
		}
		static Core::J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMdiagram>& diagram)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			JObjectFileIOHelper::StoreFsmIden(stream, diagram.Get());

			const std::vector<JUserPtr<Core::JFSMstate>>& stateVec = diagram->GetStateVec();
			const uint stateCount = (uint)stateVec.size();
			JObjectFileIOHelper::StoreAtomicData(stream, L"StateCount:", stateCount);

			for (uint i = 0; i < stateCount; ++i)
			{
				JUserPtr<JAnimationFSMstate> fsmState = diagram->GetStateByIndex(i);
				JObjectFileIOHelper::StoreFsmIden(stream, fsmState.Get());
				JObjectFileIOHelper::StoreEnumData(stream, L"StateType:", fsmState->GetStateType());
			}
			for (uint i = 0; i < stateCount; ++i)
			{
				JUserPtr<JAnimationFSMstate> state = diagram->GetStateByIndex(i);
				StateIOInterface(state)->StoreAssetData(stream, state);
			}
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		void RegisterThisPointer(JAnimationFSMdiagram* fsmDiagram)
		{
			thisPointer = GetWeakPtr(fsmDiagram);
		}
		static void RegisterTypeData()
		{
			JIdentifier::RegisterPrivateInterface(JAnimationFSMdiagram::StaticTypeInfo(), aPrivate);
			IMPL_REALLOC_BIND(JAnimationFSMdiagram::JAnimationFSMdiagramImpl, thisPointer)
		}
	};

	JAnimationFSMdiagram::InitData::InitData(Core::JFSMdiagramOwnerInterface* ownerInterface)
		:JFSMdiagram::InitData(JAnimationFSMdiagram::StaticTypeInfo(), ownerInterface)
	{}
	JAnimationFSMdiagram::InitData::InitData(const std::wstring& name, const size_t guid, Core::JFSMdiagramOwnerInterface* ownerInterface)
		: JFSMdiagram::InitData(JAnimationFSMdiagram::StaticTypeInfo(), name, guid, ownerInterface)
	{}

	Core::JIdentifierPrivate& JAnimationFSMdiagram::PrivateInterface()const noexcept
	{
		return aPrivate;
	}
	JUserPtr<JAnimationFSMstate> JAnimationFSMdiagram::GetState(const size_t stateGuid)noexcept
	{
		return Core::ConvertChildUserPtr<JAnimationFSMstate>(JFSMdiagram::GetState(stateGuid));
	}
	JUserPtr<JAnimationFSMstate> JAnimationFSMdiagram::GetStateByIndex(const uint index)noexcept
	{
		return Core::ConvertChildUserPtr<JAnimationFSMstate>(JFSMdiagram::GetStateByIndex(index));
	}
	JUserPtr<JAnimationFSMtransition> JAnimationFSMdiagram::GetTransition(const size_t transitionGuid)noexcept
	{
		const std::vector<JUserPtr<Core::JFSMstate>>& stateVec = GetStateVec();
		for (const auto& data : stateVec)
		{
			JUserPtr<Core::JFSMtransition> transition = data->GetTransition(transitionGuid);
			if (transition != nullptr)
				return Core::ConvertChildUserPtr<JAnimationFSMtransition>(transition);
		}
		return nullptr;
	}
	void JAnimationFSMdiagram::SetClip(const size_t stateGuid, const JUserPtr<JAnimationClip>& clip)noexcept
	{
		JUserPtr<JAnimationFSMstate> aniFsm = Core::ConvertChildUserPtr<JAnimationFSMstate>(GetState(stateGuid));
		if (aniFsm->GetStateType() == J_ANIMATION_STATE_TYPE::CLIP)
			static_cast<JAnimationFSMstateClip*>(aniFsm.Get())->SetClip(clip);
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

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationFSMdiagram>(*static_cast<JAnimationFSMdiagram::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		Core::JFSMdiagramPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationFSMdiagram* diagram = static_cast<JAnimationFSMdiagram*>(createdPtr);
		diagram->impl->RegisterThisPointer(diagram);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMdiagram::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<JAnimationFSMdiagram> AssetDataIOInterface::LoadAssetData(std::wifstream& stream, Core::JFSMdiagramOwnerInterface* fsmOwner)
	{
		return JAnimationFSMdiagram::JAnimationFSMdiagramImpl::LoadAssetData(stream, fsmOwner);
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMdiagram>& diagram)
	{
		return JAnimationFSMdiagram::JAnimationFSMdiagramImpl::StoreAssetData(stream, diagram);
	}

	void UpdateInterface::Initialize(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber)noexcept
	{
		diagram->impl->Initialize(updateData, layerNumber);
	}
	void UpdateInterface::Enter(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber)
	{
		diagram->impl->Enter(updateData, layerNumber);
	}
	void UpdateInterface::Update(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept
	{
		diagram->impl->Update(updateData, animationConstatns, layerNumber);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationFSMdiagramPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
}
