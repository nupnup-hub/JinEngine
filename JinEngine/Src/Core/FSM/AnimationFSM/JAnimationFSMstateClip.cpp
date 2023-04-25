#include"JAnimationFSMstateClip.h" 
#include"JAnimationFSMstateClipPrivate.h"
#include"JAnimationTime.h"
#include"JAnimationUpdateData.h"
#include"JAnimationFSMdiagram.h"
#include"JAnimationFSMtransition.h"  
#include"../../Time/JGameTimer.h" 
#include"../../File/JFileIOHelper.h" 
#include"../../Identity/JIdentifierImplBase.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../../Object/Resource/JResourceObject.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/JResourceObjectUserInterface.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static JAnimationFSMstateClipPrivate cPrivate;
		}
		 
		class JAnimationFSMstateClip::JAnimationFSMstateClipImpl : public JIdentifierImplBase,
			public JResourceObjectUserInterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMstateClipImpl)
		public:
			JAnimationFSMstateClip* thisFsmClip = nullptr;
		public:
			REGISTER_PROPERTY_EX(clip, GetClip, SetClip, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::ICON, true))
			Core::JUserPtr<JAnimationClip> clip;
		public:
			JAnimationFSMstateClipImpl(const InitData& initData, JAnimationFSMstateClip* thisFsmClip)
				:JResourceObjectUserInterface(thisFsmClip->GetGuid()), thisFsmClip(thisFsmClip)
			{
				AddEventListener(*JResourceObject::EvInterface(), thisFsmClip->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			}
			~JAnimationFSMstateClipImpl()
			{
				RemoveListener(*JResourceObject::EvInterface(), thisFsmClip->GetGuid());
			}
		public:
			Core::JUserPtr<JAnimationClip> GetClip()const noexcept
			{
				return clip;
			}
			void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept
			{
				if (clip.IsValid())
					skeletonVec.push_back(clip->GetClipSkeletonAsset().Get());
			}
		public:
			void SetClip(Core::JUserPtr<JAnimationClip> newClip)noexcept
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
				thisFsmClip->Initialize();
			}
			void Enter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
			{
				JAnimationFSMtransition* nowTransition = updateData->diagramData[layerNumber].nowTransition;
				const float timeOffset = nowTransition ? nowTransition->GetTargetStartTimeRate() : 0;

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
					updateData->lastState[updateNumber] = thisFsmClip->GetStateType();
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
				thisFsmClip->Clear();
				SetClip(Core::JUserPtr<JAnimationClip>{});
			}
		public:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
			{
				if (iden == thisFsmClip->GetGuid())
					return;

				if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
				{
					if (clip.IsValid() && jRobj->GetGuid() == clip->GetGuid())
						SetClip(Core::JUserPtr<JAnimationClip>{});
				}
			}
		public:
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JAnimationFSMstateClip::StaticTypeInfo(), cPrivate);
			}
		};

		JAnimationFSMstateClip::InitData::InitData(JUserPtr<JAnimationFSMdiagram> ownerDiagram)
			:JAnimationFSMstate::InitData(JAnimationFSMstateClip::StaticTypeInfo(), ownerDiagram)
		{}
		JAnimationFSMstateClip::InitData::InitData(const std::wstring& name, const size_t guid, JUserPtr<JAnimationFSMdiagram> ownerDiagram)
			: JAnimationFSMstate::InitData(JAnimationFSMstateClip::StaticTypeInfo(), name, guid, ownerDiagram)
		{}

		Core::JIdentifierPrivate& JAnimationFSMstateClip::GetPrivateInterface()const noexcept
		{
			return cPrivate;
		}
		J_ANIMATION_STATE_TYPE JAnimationFSMstateClip::GetStateType()const noexcept
		{
			return J_ANIMATION_STATE_TYPE::CLIP;
		}
		Core::JUserPtr<JAnimationClip> JAnimationFSMstateClip::GetClip()const noexcept
		{
			return impl->GetClip();
		}
		void JAnimationFSMstateClip::SetClip(Core::JUserPtr<JAnimationClip> newClip)noexcept
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

		using CreateInstanceInterface = JAnimationFSMstateClipPrivate ::CreateInstanceInterface;
		using DestroyInstanceInterface = JAnimationFSMstateClipPrivate::DestroyInstanceInterface;
		using AssetDataIOInterface = JAnimationFSMstateClipPrivate::AssetDataIOInterface;
		using UpdateInterface = JAnimationFSMstateClipPrivate::UpdateInterface;

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<JDITypeDataBase>&& initData)
		{
			return JPtrUtil::MakeOwnerPtr<JAnimationFSMstateClip>(*static_cast<JAnimationFSMstateClip::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMstateClip::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{ 
			static_cast<JAnimationFSMstateClip*>(ptr)->impl->Clear();
		}

		J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetData(std::wifstream& stream, JAnimationFSMstate* state)
		{
			if (!stream.is_open() || !state->GetTypeInfo().IsChildOf<JAnimationFSMstateClip>())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			J_FILE_IO_RESULT res = LoadAssetCommonData(stream, state); 
			JAnimationFSMstateClip* fsmClip = static_cast<JAnimationFSMstateClip*>(state);
			fsmClip->SetClip(JFileIOHelper::LoadHasObjectIden<JAnimationClip>(stream));
			return res;
		}
		J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, JAnimationFSMstate* state)
		{
			if (!stream.is_open() || !state->GetTypeInfo().IsChildOf<JAnimationFSMstateClip>())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			JAnimationFSMstateClip* fsmClip = static_cast<JAnimationFSMstateClip*>(state);
			J_FILE_IO_RESULT res = StoreAssetCommonData(stream, fsmClip);
			JFileIOHelper::StoreHasObjectIden(stream, fsmClip->impl->clip.Get());
			return res;
		}

		void UpdateInterface::Initialize(JFSMstate* state)noexcept
		{ 
			static_cast<JAnimationFSMstateClip*>(state)->impl->Initialize();
		}
		void UpdateInterface::Enter(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			static_cast<JAnimationFSMstateClip*>(state)->impl->Enter(updateData, layerNumber, updateNumber);
		}
		void UpdateInterface::Update(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			static_cast<JAnimationFSMstateClip*>(state)->impl->Update(updateData, layerNumber, updateNumber);
		}
		void UpdateInterface::Close(JAnimationFSMstate* state, JAnimationUpdateData* updateData)noexcept
		{
			static_cast<JAnimationFSMstateClip*>(state)->impl->Close(updateData);
		}
		void UpdateInterface::GetRegisteredSkeleton(JAnimationFSMstate* state, std::vector<JSkeletonAsset*>& skeletonVec)noexcept
		{
			static_cast<JAnimationFSMstateClip*>(state)->impl->GetRegisteredSkeleton(skeletonVec);
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
		JFSMstatePrivate::UpdateInterface& JAnimationFSMstateClipPrivate::GetUpdateInterface()const noexcept
		{
			static UpdateInterface pI;
			return pI;
		}
	}
}