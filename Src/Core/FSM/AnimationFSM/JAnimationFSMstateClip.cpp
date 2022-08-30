#include"JAnimationFSMstateClip.h" 
#include"JAnimationTime.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMtransition.h"
#include"../JFSMLoadGuidMap.h"
#include"../../GameTimer/JGameTimer.h"
#include"../../Guid/GuidCreator.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../Object/Resource/JResourceManager.h"


namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMstateClip::JAnimationFSMstateClip(const std::wstring& name, const size_t guid)
			:JAnimationFSMstate(name, guid)
		{}
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
		void JAnimationFSMstateClip::SetClip(JAnimationClip* clip)noexcept
		{
			JAnimationFSMstateClip::clip = clip;
		}
		void JAnimationFSMstateClip::Clear()noexcept
		{
			JFSMstate::Clear();
			if (clip != nullptr)
				OffResourceReference(*clip);
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::StoreContentsData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			
			if (clip != nullptr)
				stream << true << " " << clip->GetGuid() << '\n';
			else
				stream << false << " " << 0 << '\n';
			 
			return JAnimationFSMstate::StoreContentsData(stream);
		}
		std::unique_ptr<JAnimationFSMstate> JAnimationFSMstateClip::LoadIdentifierData(std::wifstream& stream, JFSMLoadGuidMap& guidMap)
		{
			if (!stream.is_open())
				return nullptr;
			
			JFSMIdentifier::JFSMIdentifierData data;
			JFSMIdentifier::LoadIdentifierData(stream, data);

			std::unique_ptr<JAnimationFSMstateClip> newState = nullptr;
			if (guidMap.isNewGuid)
			{
				newState = std::make_unique<JAnimationFSMstateClip>(data.name, MakeGuid());
				guidMap.state.emplace(data.guid, newState->GetGuid());
			}
			else
				newState = std::make_unique<JAnimationFSMstateClip>(data.name, data.guid);

			return newState;
		}
		J_FILE_IO_RESULT JAnimationFSMstateClip::LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap, IJFSMconditionStorageUser& iConditionUser)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			bool hasClip = false;
			size_t clipGuid = 0;
			
			stream >> hasClip;
			stream >> clipGuid;

			if (hasClip)
				SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(clipGuid));

			return JAnimationFSMstate::LoadContentsData(stream, guidMap, iConditionUser);
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
	}
}