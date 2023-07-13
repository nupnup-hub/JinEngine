#include"JCullingManager.h"
#include"JCullingInfo.h"

namespace JinEngine
{
	namespace Graphic
	{	 
		uint JCullingManager::GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept
		{
			return (uint)cullingInfo[(uint)type].size();
		}
		JUserPtr<JCullingInfo> JCullingManager::GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept
		{
			return cullingInfo[(uint)type][index];
		}
		std::vector<JUserPtr<JCullingInfo>> JCullingManager::GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept
		{
			const uint count = (uint)cullingInfo[(uint)type].size();
			std::vector<JUserPtr<JCullingInfo>> user(count);
			for (uint i = 0; i < count; ++i)
				user[i] = cullingInfo[(uint)type][i];
			return user;
		}
		JUserPtr<JCullingInfo> JCullingManager::CreateFrsutumData()
		{
			return CreateCullingInfo(J_CULLING_TYPE::FRUSTUM);
		}
		JUserPtr<JCullingInfo> JCullingManager::CreateOcclusionData()
		{
			return CreateCullingInfo(J_CULLING_TYPE::OCCLUSION);
		}
		JUserPtr<JCullingInfo> JCullingManager::CreateCullingInfo(const J_CULLING_TYPE type)
		{
			const uint infoIndex = cullingInfo[(uint)type].size();
			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, type);

			ownerPtr->SetArrayIndex(infoIndex); 
			cullingInfo[(uint)type].push_back(std::move(ownerPtr));
			return cullingInfo[(uint)type][infoIndex];
		}
		bool JCullingManager::DestroyCullingData(JCullingInfo* info)
		{
			if (info == nullptr)
				return false;

			const J_CULLING_TYPE type = info->GetCullingType();
			const uint count = cullingInfo[(uint)type].size();

			const uint index = info->GetArrayIndex();
			for (uint i = index + 1; i < count; ++i)
				cullingInfo[(uint)type][i]->SetArrayIndex(i - 1);

			cullingInfo[(uint)type].erase(cullingInfo[(uint)type].begin() + index);
			info = nullptr;
			return true;
		}	  
		void JCullingManager::Clear()
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				cullingInfo[i].clear();
		}
		void JCullingManager::RegisterTypeData()
		{
			using JAllocationDesc = JinEngine::Core::JAllocationDesc;
			using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
			using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
			using ReceiverPtr = JAllocationDesc::ReceiverPtr;
			using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
			using MemIndex = JAllocationDesc::MemIndex;

			NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{
				JCullingInfo* movedInfo = static_cast<JCullingInfo*>(movedPtr);
				JCullingManager* manager = movedInfo->manager;

				manager->cullingInfo[(int)movedInfo->GetCullingType()][movedInfo->GetArrayIndex()].Release();
				manager->cullingInfo[(int)movedInfo->GetCullingType()][movedInfo->GetArrayIndex()].Reset(movedInfo);
			};
			auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
			std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

			desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			JCullingInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
		}
	}
}