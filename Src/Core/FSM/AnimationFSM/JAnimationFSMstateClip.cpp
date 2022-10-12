#include"JAnimationFSMstateClip.h" 
#include"JAnimationTime.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMtransition.h" 
#include"../JFSMfactory.h" 
#include"../../Time/JGameTimer.h" 
#include"../../File/JFileIOHelper.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"
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
		void JAnimationFSMstateClip::Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept
		{
			clip->ClipEnter(animationTime, animationShareData, srcSkeletonAsset, JGameTimer::Instance().TotalTime(), timeOffset);
		}
		void JAnimationFSMstateClip::Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept
		{
			animationShareData.ClearSkeletonBlendRate(updateNumber);
			if (clip != nullptr)
			{
				clip->Update(animationTime,
					animationShareData,
					srcSkeletonAsset,
					animationShareData.localTransform[updateNumber],
					JGameTimer::Instance().TotalTime(),
					JGameTimer::Instance().DeltaTime());

				size_t clipGuid = clip->GetClipSkeletonAsset()->GetGuid();
				animationShareData.skeletonBlendRate[updateNumber][clipGuid] = 1;
				animationShareData.lastState[updateNumber] = GetStateType();
			}
			else
				animationShareData.StuffIdentity(updateNumber);
		}
		void JAnimationFSMstateClip::Close(JAnimationShareData& animationShareData)noexcept
		{
			clip->ClipClose();
		}
		void JAnimationFSMstateClip::GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept
		{
			if (clip != nullptr)
				skeletonVec.push_back(clip->GetClipSkeletonAsset());
		}
		void JAnimationFSMstateClip::SetClip(JAnimationClip* newClip)noexcept
		{
			CallOffResourceReference(clip);
			clip = newClip;
			CallOnResourceReference(clip);
		}
		void JAnimationFSMstateClip::Clear()noexcept
		{
			JFSMstate::Clear();
			SetClip(nullptr);
		}
		void JAnimationFSMstateClip::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (clip != nullptr && jRobj->GetGuid() == clip->GetGuid())
					SetClip(nullptr);
			}
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			J_FILE_IO_RESULT res = JAnimationFSMstate::StoreData(stream);
			JFileIOHelper::StoreHasObjectIden(stream, clip);
			return res;
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::LoadData(std::wifstream& stream, IJFSMconditionStorageUser& iConditionUser)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			J_FILE_IO_RESULT res = JAnimationFSMstate::LoadData(stream, iConditionUser);
			Core::JIdentifier* clip = JFileIOHelper::LoadHasObjectIden(stream);
			if (clip != nullptr && clip->GetTypeInfo().IsA(JAnimationClip::StaticTypeInfo()))
				SetClip(static_cast<JAnimationClip*>(clip));

			return res;
		}
		void JAnimationFSMstateClip::RegisterJFunc()
		{
			auto createStateLam = [](JOwnerPtr<JFSMstateInitData> initData)-> JFSMstate*
			{
				if (initData.IsValid())
				{
					JOwnerPtr<JAnimationFSMstateClip> ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationFSMstateClip>(*initData.Get());
					JAnimationFSMstateClip* newState = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newState;
				}
				return nullptr;
			};
			JFSFI<JAnimationFSMstateClip>::RegisterState(createStateLam);
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