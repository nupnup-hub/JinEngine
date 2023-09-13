#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMstateClipPrivate.h"
#include"JAnimationTime.h"
#include"JAnimationUpdateData.h"
#include"JAnimationFSMdiagram.h"
#include"JAnimationFSMtransition.h"  
#include"../../Skeleton/JSkeletonAsset.h"
#include"../../AnimationClip/JAnimationClip.h" 
#include"../../JResourceManager.h"
#include"../../JResourceObjectUserInterface.h"
#include"../../../../Core/Time/JGameTimer.h" 
#include"../../../../Core/File/JFileIOHelper.h" 
#include"../../../../Core/Reflection/JTypeImplBase.h"
#include<fstream>

namespace JinEngine
{

	namespace
	{
		static JAnimationFSMstateClipPrivate cPrivate;
	}

	class JAnimationFSMstateClip::JAnimationFSMstateClipImpl : public Core::JTypeImplBase,
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMstateClipImpl)
	public:
		JWeakPtr<JAnimationFSMstateClip> thisPointer = nullptr;
	public:
		REGISTER_PROPERTY_EX(clip, GetClip, SetClip, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::ICON, true))
		JUserPtr<JAnimationClip> clip;
	public:
		JAnimationFSMstateClipImpl(const InitData& initData, JAnimationFSMstateClip* thisFsmClipRaw)
		{}
		~JAnimationFSMstateClipImpl() {}
	public:
		JUserPtr<JAnimationClip> GetClip()const noexcept
		{
			return clip;
		}
		void GetRegisteredSkeleton(std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept
		{
			if (clip.IsValid())
				skeletonVec.push_back(clip->GetClipSkeletonAsset());
		}
	public:
		void SetClip(const JUserPtr<JAnimationClip>& newClip)noexcept
		{
			if (clip.IsValid())
				CallOffResourceReference(clip.Get());
			clip = newClip;
			if (clip.IsValid())
				CallOnResourceReference(clip.Get());
		}
	public:
		bool CanLoop()const noexcept
		{
			return clip->IsLoop();
		}
	public:
		void Initialize()noexcept
		{
			thisPointer->Initialize();
		}
		void Enter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			JUserPtr<JAnimationFSMtransition> nowTransition = updateData->diagramData[layerNumber].nowTransition;
			const float timeOffset = nowTransition.IsValid() ? nowTransition->GetTargetStartTimeRate() : 0;

			if (clip.IsValid() && clip->GetClipSkeletonAsset().IsValid())
				clip->ClipEnter(updateData, layerNumber, updateNumber, timeOffset);
		}
		void Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			updateData->ClearSkeletonBlendRate(updateNumber);
			if (clip.IsValid() && clip->GetClipSkeletonAsset().IsValid())
			{
				clip->Update(updateData, layerNumber, updateNumber);
				size_t clipGuid = clip->GetClipSkeletonAsset()->GetGuid();
				updateData->skeletonBlendRate[updateNumber][clipGuid] = 1;
				updateData->lastState[updateNumber] = thisPointer->GetStateType();
			}
			else
				updateData->StuffIdentity(layerNumber, updateNumber);
		}
		void Close(JAnimationUpdateData* updateData)noexcept
		{
			clip->ClipClose();
		}
	public:
		void Clear()noexcept
		{
			thisPointer->Clear();
			SetClip(JUserPtr<JAnimationClip>{});
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (clip.IsValid() && jRobj->GetGuid() == clip->GetGuid())
					SetClip(JUserPtr<JAnimationClip>{});
			}
		}
	public:
		void NotifyReAlloc()
		{
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void RegisterThisPointer(JAnimationFSMstateClip* clip)
		{
			thisPointer = GetWeakPtr(clip);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
		static void RegisterTypeData()
		{
			JIdentifier::RegisterPrivateInterface(JAnimationFSMstateClip::StaticTypeInfo(), cPrivate);
			IMPL_REALLOC_BIND(JAnimationFSMstateClip::JAnimationFSMstateClipImpl, thisPointer)
		}
	};

	JAnimationFSMstateClip::InitData::InitData(const JUserPtr<JAnimationFSMdiagram>& ownerDiagram)
		:JAnimationFSMstate::InitData(JAnimationFSMstateClip::StaticTypeInfo(), ownerDiagram)
	{}
	JAnimationFSMstateClip::InitData::InitData(const std::wstring& name, const size_t guid, const JUserPtr<JAnimationFSMdiagram>& ownerDiagram)
		: JAnimationFSMstate::InitData(JAnimationFSMstateClip::StaticTypeInfo(), name, guid, ownerDiagram)
	{}

	Core::JIdentifierPrivate& JAnimationFSMstateClip::PrivateInterface()const noexcept
	{
		return cPrivate;
	}
	J_ANIMATION_STATE_TYPE JAnimationFSMstateClip::GetStateType()const noexcept
	{
		return J_ANIMATION_STATE_TYPE::CLIP;
	}
	JUserPtr<JAnimationClip> JAnimationFSMstateClip::GetClip()const noexcept
	{
		return impl->GetClip();
	}
	void JAnimationFSMstateClip::SetClip(const JUserPtr<JAnimationClip>& newClip)noexcept
	{
		impl->SetClip(newClip);
	}
	bool JAnimationFSMstateClip::CanLoop()const noexcept
	{
		return impl->CanLoop();
	}
	JAnimationFSMstateClip::JAnimationFSMstateClip(const InitData& initData)
		:JAnimationFSMstate(initData), impl(std::make_unique<JAnimationFSMstateClipImpl>(initData, this))
	{ }
	JAnimationFSMstateClip::~JAnimationFSMstateClip()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationFSMstateClipPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JAnimationFSMstateClipPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JAnimationFSMstateClipPrivate::AssetDataIOInterface;
	using UpdateInterface = JAnimationFSMstateClipPrivate::UpdateInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationFSMstateClip>(*static_cast<JAnimationFSMstateClip::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JAnimationFSMstatePrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationFSMstateClip* clip = static_cast<JAnimationFSMstateClip*>(createdPtr);
		clip->impl->RegisterThisPointer(clip);
		clip->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMstateClip::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JAnimationFSMstateClip*>(ptr)->impl->DeRegisterPreDestruction();
		static_cast<JAnimationFSMstateClip*>(ptr)->impl->Clear();
		JAnimationFSMstatePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!stream.is_open() || !state->GetTypeInfo().IsChildOf<JAnimationFSMstateClip>())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		Core::J_FILE_IO_RESULT res = LoadAssetCommonData(stream, state);
		JAnimationFSMstateClip* fsmClip = static_cast<JAnimationFSMstateClip*>(state.Get());
		fsmClip->SetClip(JFileIOHelper::LoadHasObjectIden<JAnimationClip>(stream));
		return res;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!stream.is_open() || !state->GetTypeInfo().IsChildOf<JAnimationFSMstateClip>())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JUserPtr<JAnimationFSMstateClip> fsmClip = Core::ConnectChildUserPtr<JAnimationFSMstateClip>(state);
		Core::J_FILE_IO_RESULT res = StoreAssetCommonData(stream, fsmClip);
		JFileIOHelper::StoreHasObjectIden(stream, fsmClip->impl->clip.Get());
		return res;
	}

	void UpdateInterface::Initialize(const JUserPtr<Core::JFSMstate>& state)noexcept
	{
		static_cast<JAnimationFSMstateClip*>(state.Get())->impl->Initialize();
	}
	void UpdateInterface::Enter(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
	{
		static_cast<JAnimationFSMstateClip*>(state.Get())->impl->Enter(updateData, layerNumber, updateNumber);
	}
	void UpdateInterface::Update(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
	{
		static_cast<JAnimationFSMstateClip*>(state.Get())->impl->Update(updateData, layerNumber, updateNumber);
	}
	void UpdateInterface::Close(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData)noexcept
	{
		static_cast<JAnimationFSMstateClip*>(state.Get())->impl->Close(updateData);
	}
	void UpdateInterface::GetRegisteredSkeleton(const JUserPtr<JAnimationFSMstate>& state, std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept
	{
		static_cast<JAnimationFSMstateClip*>(state.Get())->impl->GetRegisteredSkeleton(skeletonVec);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationFSMstateClipPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JAnimationFSMstateClipPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JAnimationFSMstatePrivate::AssetDataIOInterface& JAnimationFSMstateClipPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
	Core::JFSMstatePrivate::UpdateInterface& JAnimationFSMstateClipPrivate::GetUpdateInterface()const noexcept
	{
		static UpdateInterface pI;
		return pI;
	}
}
