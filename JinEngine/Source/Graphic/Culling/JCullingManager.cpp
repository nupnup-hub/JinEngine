#include"JCullingManager.h"
#include"JCullingInfo.h"  
#include"JCullingResultHolder.h" 
#include"../Device/JGraphicDevice.h"
namespace JinEngine
{
	namespace Graphic
	{	 		
		/*	
		* create resource senario
		1. Create resource and info(Owner Pointer)
		2. caller(impl) is derived interface(has cinfo and access cmanager public method)
		3. impl interface class can derived user access class(restrict version impl derived interface)
		*/
		uint JCullingManager::GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept
		{
			return (uint)cullingInfo[(uint)type].size();
		}
		JUserPtr<JCullingInfo> JCullingManager::GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept
		{
			return cullingInfo[(uint)type].size() > index ? JUserPtr<JCullingInfo>{cullingInfo[(uint)type][index]} : nullptr;
		}  
		std::vector<JUserPtr<JCullingInfo>> JCullingManager::GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept
		{
			const uint count = (uint)cullingInfo[(uint)type].size();
			std::vector<JUserPtr<JCullingInfo>> user(count);
			for (uint i = 0; i < count; ++i)
				user[i] = cullingInfo[(uint)type][i];
			return user;
		}
		JCullingResultHolder* JCullingManager::GetHolder(JCullingInfo* info)const noexcept
		{
			return info != nullptr ? info->resultHolder.get() : nullptr;
		}
		std::vector<JOwnerPtr<JCullingInfo>>& JCullingManager::GetCullinginfoRefVec(const J_CULLING_TYPE type)noexcept
		{
			return cullingInfo[(uint)type];
		}
		void JCullingManager::ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity)
		{
			auto& typeVec = cullingInfo[(uint)type];
			const uint count = (uint)cullingInfo[(uint)type].size();
			for (uint i = 0; i < count; ++i)
				ReBuildBuffer(typeVec[i].Get(), device, capacity, i);		
		}
		void JCullingManager::ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity, const J_CULLING_TARGET bufferTarget)
		{
			auto& typeVec = cullingInfo[(uint)type];
			const uint count = (uint)cullingInfo[(uint)type].size();
			for (uint i = 0; i < count; ++i)
			{
				if(Core::HasSQValueEnum(typeVec[i]->GetCullingTarget(), bufferTarget))
					ReBuildBuffer(typeVec[i].Get(), device, capacity, i);
			}
		}
		JUserPtr<JCullingInfo> JCullingManager::PostCreation(JOwnerPtr<JCullingInfo>&& newInfo, const J_CULLING_TYPE type)
		{
			const uint infoIndex = (uint)cullingInfo[(uint)type].size();
			newInfo->SetArrayIndex(infoIndex);
			cullingInfo[(uint)type].push_back(std::move(newInfo));
			return cullingInfo[(uint)type][infoIndex];
		}
		bool JCullingManager::DestroyCullingData(JCullingInfo* info, JGraphicDevice* device)
		{
			if (info == nullptr)
				return false;

			bool startCommandThisCreation = false;
			if (device->CanStartPublicCommand())
			{
				device->FlushCommandQueue();
				device->StartPublicCommand();
				startCommandThisCreation = true;
			}

			const J_CULLING_TYPE type = info->GetCullingType();
			const uint count = (uint)cullingInfo[(uint)type].size();

			const uint index = info->GetArrayIndex();
			for (uint i = index + 1; i < count; ++i)
				cullingInfo[(uint)type][i]->SetArrayIndex(i - 1);

			cullingInfo[(uint)type].erase(cullingInfo[(uint)type].begin() + index);
			info = nullptr;

			PostDestruction(type, index);
			if (startCommandThisCreation)
			{
				device->EndPublicCommand();
				device->FlushCommandQueue();
			}
			return true;
		} 
		void JCullingManager::Clear()
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				cullingInfo[i].clear();
		}
		void JCullingManager::RegisterTypeData()
		{
			//Caution!
			//Culling Info은 Device에 따른 상속을 사용하지않으므로 
			//JCullingManager(Base)에서 Realloc 함수를 등록한다
			//추후에 상속을 사용할시 해당 device를 사용하는 JCullingManager에서 
			//Realloc을 등록하도록한다.
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

				//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
				//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
				//Release() 한다음 Reset()을 호출해야한다.
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