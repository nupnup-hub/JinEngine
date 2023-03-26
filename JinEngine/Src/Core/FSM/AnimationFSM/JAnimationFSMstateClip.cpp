#include"JAnimationFSMstateClip.h" 
#include"JAnimationTime.h"
#include"JAnimationUpdateData.h"
#include"JAnimationFSMtransition.h" 
#include"../JFSMfactory.h" 
#include"../../Time/JGameTimer.h" 
#include"../../File/JFileIOHelper.h" 
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/JResourceManager.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		void JAnimationFSMstateClip::Initialize()noexcept
		{
			JFSMstate::Initialize();
		}
		J_ANIMATION_STATE_TYPE JAnimationFSMstateClip::GetStateType()const noexcept
		{
			return J_ANIMATION_STATE_TYPE::CLIP;
		} 
		void JAnimationFSMstateClip::Enter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			JAnimationFSMtransition* nowTransition = updateData->diagramData[layerNumber].nowTransition;
			const float timeOffset = nowTransition ? nowTransition->GetTargetStartTimeRate() : 0;

			if (clip.IsValid() && clip->GetClipSkeletonAsset().IsValid())
				clip->ClipEnter(updateData, layerNumber, updateNumber, timeOffset);
		}
		void JAnimationFSMstateClip::Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			updateData->ClearSkeletonBlendRate(updateNumber);
			if (clip.IsValid() && clip->GetClipSkeletonAsset().IsValid())
			{ 
				clip->Update(updateData, layerNumber, updateNumber);
				size_t clipGuid = clip->GetClipSkeletonAsset()->GetGuid();
				updateData->skeletonBlendRate[updateNumber][clipGuid] = 1;
				updateData->lastState[updateNumber] = GetStateType();
			}
			else
				updateData->StuffIdentity(layerNumber, updateNumber);
		}
		void JAnimationFSMstateClip::Close(JAnimationUpdateData* updateData)noexcept
		{
			clip->ClipClose();
		}
		void JAnimationFSMstateClip::GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept
		{
			if (clip.IsValid())
				skeletonVec.push_back(clip->GetClipSkeletonAsset().Get());
		}
		Core::JUserPtr<JAnimationClip> JAnimationFSMstateClip::GetClip()const noexcept
		{
			return clip;
		}
		void JAnimationFSMstateClip::SetClip(Core::JUserPtr<JAnimationClip> newClip)noexcept
		{
			if(clip.IsValid())
				CallOffResourceReference(clip.Get());
			clip = newClip;
			if (clip.IsValid())
				CallOnResourceReference(clip.Get());
		}
		bool JAnimationFSMstateClip::CanLoop()const noexcept
		{
			return clip->IsLoop();
		}
		void JAnimationFSMstateClip::Clear()noexcept
		{
			JFSMstate::Clear();
			SetClip(Core::JUserPtr<JAnimationClip>{});
		}
		void JAnimationFSMstateClip::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (clip.IsValid() && jRobj->GetGuid() == clip->GetGuid())
					SetClip(Core::JUserPtr<JAnimationClip>{});
			}
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			J_FILE_IO_RESULT res = JAnimationFSMstate::StoreData(stream);
			JFileIOHelper::StoreHasObjectIden(stream, clip.Get());
			return res;
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::LoadData(std::wifstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			J_FILE_IO_RESULT res = JAnimationFSMstate::LoadData(stream);
			Core::JIdentifier* clip = JFileIOHelper::LoadHasObjectIden(stream);
			if (clip != nullptr && clip->GetTypeInfo().IsA(JAnimationClip::StaticTypeInfo()))
				SetClip(Core::GetUserPtr<JAnimationClip>(clip));

			return res;
		}
		void JAnimationFSMstateClip::RegisterCallOnce()
		{
			auto createClipLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::STATE)
				{
					JFSMstateInitData* stateInitData = static_cast<JFSMstateInitData*>(initData.Get());
					JOwnerPtr<JAnimationFSMstateClip> ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationFSMstateClip>(*stateInitData);
					JAnimationFSMstateClip* newState = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newState;
				}
				return nullptr;
			};
			JFFI<JAnimationFSMstateClip>::Register(createClipLam);
		}
		JAnimationFSMstateClip::JAnimationFSMstateClip(const JFSMstateInitData& initData)
			:JAnimationFSMstate(initData)
		{
			AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		JAnimationFSMstateClip::~JAnimationFSMstateClip()
		{
			RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
		}
	}
}