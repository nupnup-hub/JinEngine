/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#include"JDx12CsmManager.h"
#include"JDx12CsmHandlerInfo.h"
#include"JDx12CsmTargetInfo.h"

namespace JinEngine::Graphic
{
	JDx12CsmManager::~JDx12CsmManager()
	{

	}
	void JDx12CsmManager::Initialize(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		JCsmManager::Initialize(device);
	}
	void JDx12CsmManager::Clear()
	{
		ClearResource();
		JCsmManager::Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12CsmManager::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	JOwnerPtr<JCsmHandlerInfo> JDx12CsmManager::_CreateHandler(JCsmHandleCreationDesc& desc, JCsmAreaInfo* areInfo)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JDx12CsmHandlerInfo>(this, areInfo, std::move(desc.notifyAddCsmTargetB), std::move(desc.notifySubtractCsmTargetB));
	}
	JOwnerPtr<JCsmTargetInfo> JDx12CsmManager::_CreateTarget(JCsmTargetCreationDesc& desc, JCsmAreaInfo* areInfo)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JDx12CsmTargetInfo>(this, areInfo, std::move(desc.getBoundingFrustumB));
	}
	void JDx12CsmManager::BuildResource(JGraphicDevice* device)
	{

	}
	void JDx12CsmManager::ClearResource()
	{

	}
	void JDx12CsmManager::RegisterTypeData()
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


		NotifyReAllocPtr handlerNotifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
		{
			JDx12CsmHandlerInfo* movedInfo = static_cast<JDx12CsmHandlerInfo*>(movedPtr);
			JDx12CsmManager* manager = movedInfo->manager;

			JCsmManager::AreaData& vec = *manager->GetAreaDataPointer(movedInfo->GetAreaInfo()->GetGuid());
			
			//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
			//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
			//Release() 한다음 Reset()을 호출해야한다. 
			//2024-08-15 수정 포인터만 변경하는 Swap 사용 
			vec.handler[movedInfo->GetIndex()].Swap(movedInfo);
		};
		auto handlerReAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(handlerNotifyPtr);
		std::unique_ptr<JAllocationDesc> handlerDesc = std::make_unique<JAllocationDesc>();

		handlerDesc->notifyReAllocB = UniqueBind(std::move(handlerReAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12CsmTargetInfo::StaticTypeInfo().SetAllocationOption(std::move(handlerDesc));

		NotifyReAllocPtr targetNotifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
		{
			JDx12CsmTargetInfo* movedInfo = static_cast<JDx12CsmTargetInfo*>(movedPtr);
			JDx12CsmManager* manager = movedInfo->manager;

			JCsmManager::AreaData& vec = *manager->GetAreaDataPointer(movedInfo->GetAreaInfo()->GetGuid());
	 
			//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
			//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
			//Release() 한다음 Reset()을 호출해야한다.
			//2024-08-15 수정 포인터만 변경하는 Swap 사용 
			vec.target[movedInfo->GetIndex()].Swap(movedInfo);
		};
		auto targetReAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(targetNotifyPtr);
		std::unique_ptr<JAllocationDesc> targetDesc = std::make_unique<JAllocationDesc>();

		targetDesc->notifyReAllocB = UniqueBind(std::move(targetReAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12CsmTargetInfo::StaticTypeInfo().SetAllocationOption(std::move(targetDesc));
	}
}