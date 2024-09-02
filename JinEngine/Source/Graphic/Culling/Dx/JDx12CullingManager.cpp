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


#include"JDx12CullingManager.h"
#include"JDx12CullingInfo.h"
#include"JDx12CullingResourceHolder.h"
#include"../JCullingInfo.h"  
#include"../JCullingInterface.h"
#include"../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
#include"../../../Core/Log/JLogMacro.h"
#ifdef DEVELOP
#include"../../../Develop/Debug/JDevelopDebug.h"
#endif
namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
#if defined (USE_DEBUG) && defined(DEVELOP)
			static constexpr bool allowDebug = false; 
#else
			static constexpr const bool allowDebug = false;
#endif

			struct BuildSet
			{
			public:
				JDx12GraphicDevice* device = nullptr;
				ID3D12GraphicsCommandList* cmdList = nullptr;
				Microsoft::WRL::ComPtr<ID3D12Resource> upload;
				bool startCommandThisFunc = false;
			public:
				BuildSet(JGraphicDevice* d)
				{
					device = static_cast<JDx12GraphicDevice*>(d);
					cmdList = device->GetPublicCmdList();
				}
			};
			//query heap count = initOccQueryHeapCapacity <= newCapacity
			static void BuildOccQueryHeaps(JGraphicDevice* device, const size_t capacity, Microsoft::WRL::ComPtr<ID3D12QueryHeap>& heap)
			{
				if (device == nullptr || device->GetDeviceType() != J_GRAPHIC_DEVICE_TYPE::DX12)
					return;

				heap = nullptr;
				ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
				D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
				queryHeapDesc.Count = (uint)capacity;
				queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
				ThrowIfFailedHr(d3d12Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&heap)));
				heap->SetName(L"Occlusion Query Heap ");			 
			}
		}
	 
		JDx12CullingManager::~JDx12CullingManager()
		{
			ClearResource();
		}
		void JDx12CullingManager::Initialize(JGraphicDevice* device)
		{
			if (!IsSameDevice(device))
				return;

			JCullingManager::Initialize(device);
		}
		void JDx12CullingManager::Clear()
		{
			ClearResource();
			JCullingManager::Clear();
		}
		J_GRAPHIC_DEVICE_TYPE JDx12CullingManager::GetDeviceType()const noexcept
		{
			return J_GRAPHIC_DEVICE_TYPE::DX12;
		}
		JDx12CullingResourceHolder* JDx12CullingManager::GetDxHolder(const J_CULLING_TYPE type, int index)const noexcept
		{
			auto info = GetCullingInfo(type, index);
			if (info == nullptr || !info->IsCullingResultInGpu())
				return nullptr;

			return static_cast<JDx12CullingResourceHolder*>(GetHolder(info.Get()));
		}
		ID3D12Resource* JDx12CullingManager::GetResource(const J_CULLING_TYPE type, int index)const noexcept
		{
			auto info = GetCullingInfo(type, index);
			if (info == nullptr || !info->IsCullingResultInGpu())
				return nullptr;

			return static_cast<JDx12CullingResourceHolder*>(GetHolder(info.Get()))->GetResource();
		}
		void JDx12CullingManager::UpdateFrameResourceIndex(const uint frameIndex)
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
			{
				auto& refVec = GetCullinginfoRefVec((J_CULLING_TYPE)i);
				for (const auto& data : refVec)
				{
					auto holder = GetHolder(data.Get());
					if (holder->GetBufferType() == J_GRAPHIC_BUFFER_TYPE::READ_BACK)
						static_cast<JDx12ReadBackResourceInterface*>(holder)->SetCurrent(frameIndex);
				}		 
			}
		}
		void JDx12CullingManager::CopyCullingResultGpuToCpu(JGraphicDevice* device, const uint frameIndex)
		{  
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
			{
				const J_CULLING_TYPE cType = (J_CULLING_TYPE)i;
				auto& refVec = GetCullinginfoRefVec(cType);
				for (const auto& data : refVec)
				{
					auto updateInfo = data->GetUpdateddInfo(frameIndex);
					auto holder = GetHolder(data.Get());
					if (holder->GetBufferType() != J_GRAPHIC_BUFFER_TYPE::READ_BACK)
						continue;

					auto readbackHolder = static_cast<JDx12ReadBackResourceInterface*>(holder);
					if (updateInfo.updatedCount > 0)
					{  
						readbackHolder->CopyOnCpuBuffer(updateInfo.updatedStartIndex, updateInfo.updatedCount);
						updateInfo.updatedStartIndex = 0;
						updateInfo.updatedCount = 0;
						data->SetUpdatedInfo(updateInfo, frameIndex);
					}
				}
			}
		}
		void JDx12CullingManager::StuffClearValue(JGraphicDevice* device, const J_CULLING_TYPE type)
		{ 
			auto& refVec = GetCullinginfoRefVec(type);
			for (const auto& data : refVec)
			{ 
				auto holder = GetHolder(data.Get());
				if (holder->GetBufferType() != J_GRAPHIC_BUFFER_TYPE::READ_BACK)
					continue;

				BuildSet data(device);
				device->StartPublicCommandSet(data.startCommandThisFunc);

				auto readbackHolder = static_cast<JDx12ReadBackResourceInterface*>(holder);
				readbackHolder->SutffClearValue(data.cmdList, data.upload.Get());

				device->EndPublicCommandSet(data.startCommandThisFunc);
				if (!data.startCommandThisFunc)
					device->ReStartPublicCommandSet();
			}
		}
		void JDx12CullingManager::ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index)
		{
			if (info == nullptr)
				return;

			BuildSet data(device); 
			auto holder = GetHolder(info);
			if (holder->IsGpuResource())
				device->StartPublicCommandSet(data.startCommandThisFunc);
			 
			const J_CULLING_TYPE cType = info->GetCullingType();
			switch (cType)
			{
			case JinEngine::J_CULLING_TYPE::FRUSTUM:
			{
				if (info->IsCullingResultInGpu())
				{
					static_cast<JDx12FrustumCullingResultHolder*>(holder)->Build(device, capacity);
					static_cast<JDx12FrustumCullingResultHolder*>(holder)->SutffClearValue(data.cmdList, data.upload.Get());
				}
				else
					static_cast<JFrustumCullingResultHolder*>(holder)->Build(capacity);
					
				break;
			}
			case JinEngine::J_CULLING_TYPE::HZB_OCCLUSION:
			{
				static_cast<JHzbDx12CullingResultHolder*>(holder)->Build(device, capacity);
				static_cast<JHzbDx12CullingResultHolder*>(holder)->SutffClearValue(data.cmdList, data.upload.Get());
				break;
			}
			case JinEngine::J_CULLING_TYPE::HD_OCCLUSION:
			{  
				BuildOccQueryHeaps(device, capacity, occQueryHeap[index]);
				static_cast<JHdDx12CullingResultHolder*>(holder)->Build(device, capacity);
				static_cast<JHdDx12CullingResultHolder*>(holder)->SutffClearValue(data.cmdList, data.upload.Get());
				break;
			}  
			default:
				break;
			}
			if (holder->IsGpuResource())
			{
				device->EndPublicCommandSet(data.startCommandThisFunc);
				if (!data.startCommandThisFunc)
					device->ReStartPublicCommandSet();
			}
			/**
			* 새로운 buffer에 capacity를 updateInfo에 반영하지않으면
			* CopyOnCpuBuffer시에 잘못된 범위에 메모리를 참조하게된다.
			*/
			const uint updateDataCount = info->GetUpdatedInfoCount();
			for (uint i = 0; i < updateDataCount; ++i)
			{
				auto updateInfo = info->GetUpdateddInfo(i);
				const uint elementCount = info->GetResultBufferElementCount();
				if (updateInfo.updatedStartIndex >= elementCount)
				{				
					//over index는 전부 무효처리
					updateInfo.updatedStartIndex = 0;
					updateInfo.updatedCount = 0;
				}
				else if (updateInfo.updatedCount + updateInfo.updatedStartIndex >= elementCount)
					updateInfo.updatedCount = elementCount - updateInfo.updatedStartIndex;
				info->SetUpdatedInfo(updateInfo, i);
			}
		}
		ID3D12QueryHeap* JDx12CullingManager::GetQueryHeap(const uint index)const noexcept
		{
			return occQueryHeap.size() > index ? occQueryHeap[index].Get() : nullptr;
		}
		bool JDx12CullingManager::HasDependency(const JGraphicInfo::TYPE type)const noexcept
		{
			if (type == JGraphicInfo::TYPE::FRAME)
				return true;
			else
				return false;
		}
		bool JDx12CullingManager::HasDependency(const JGraphicOption::TYPE type)const noexcept
		{
			if (type == JGraphicOption::TYPE::CULLING)
				return true;
			else
				return false;
		}
		void JDx12CullingManager::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
		{
			auto dx12Set = static_cast<const JDx12GraphicInfoChangedSet&>(set);
			auto& preFrame = dx12Set.preInfo.frame;
			auto& newFrame = dx12Set.newInfo.frame;
			if (preFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT) != newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT) ||
				preFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT) != newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT) ||
				preFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT) != newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT))
			{
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, dx12Set.device, newFrame.GetLocalLightCapacity(), J_CULLING_TARGET::LIGHT);
			}
			if (preFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT) != newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT))
			{ 
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, dx12Set.device, newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT), J_CULLING_TARGET::RENDERITEM);
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::HD_OCCLUSION, dx12Set.device, newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT));
			}
			if (preFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT) != newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT))
			{ 
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::HZB_OCCLUSION, dx12Set.device, newFrame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT));
			}
		}
		void JDx12CullingManager::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
		{ 
			auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
			if (set.preOption.culling.isOcclusionQueryActivated != set.newOption.culling.isOcclusionQueryActivated)
			{
				StuffClearValue(dx12Set.device, J_CULLING_TYPE::HZB_OCCLUSION);
				StuffClearValue(dx12Set.device, J_CULLING_TYPE::HD_OCCLUSION);
			}
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateFrsutumData(JGraphicDevice* device, const JCullingCreationDesc& desc)
		{
			if (!IsSameDevice(device))
				return nullptr;
	 
			BuildSet data(device);
			JOwnerPtr<JCullingInfo> ownerPtr; 
			if (desc.useGpu)
			{ 
				device->StartPublicCommandSet(data.startCommandThisFunc);
				auto resultHolder = std::make_unique<JDx12FrustumCullingResultHolder>(desc.target, desc.currFrameIndex);
				resultHolder->Build(device, desc.capacity);
				resultHolder->SutffClearValue(data.cmdList, data.upload.Get());
				ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JDx12CullingInfo>(J_CULLING_TYPE::FRUSTUM, Constants::gNumFrameResources, std::move(resultHolder), this);
			}
			else
			{
				auto resultHolder = std::make_unique<JFrustumCullingResultHolder>(desc.target);
				resultHolder->Build(desc.capacity);
				ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JDx12CullingInfo>(J_CULLING_TYPE::FRUSTUM, Constants::gNumFrameResources, std::move(resultHolder), this);
			}	  
			auto user = PostCreation(std::move(ownerPtr), J_CULLING_TYPE::FRUSTUM);
			
			device->EndPublicCommandSet(data.startCommandThisFunc);
			if (desc.useGpu && user != nullptr && !data.startCommandThisFunc)
				device->ReStartPublicCommandSet();
			return user;
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateHzbOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc)
		{
			if (!IsSameDevice(device))
				return nullptr;

			BuildSet data(device);
			device->StartPublicCommandSet(data.startCommandThisFunc);

			auto dx12Holder = std::make_unique<JHzbDx12CullingResultHolder>(desc.target, desc.currFrameIndex);
			dx12Holder->Build(device, desc.capacity); 
			dx12Holder->SutffClearValue(data.cmdList, data.upload.Get());

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JDx12CullingInfo>(J_CULLING_TYPE::HZB_OCCLUSION, Constants::gNumFrameResources, std::move(dx12Holder), this);
			auto user = PostCreation(std::move(ownerPtr), J_CULLING_TYPE::HZB_OCCLUSION);

			device->EndPublicCommandSet(data.startCommandThisFunc);
			if (user != nullptr && !data.startCommandThisFunc)
				device->ReStartPublicCommandSet();
			return user;
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateHdOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc)
		{
			if (!IsSameDevice(device))
				return nullptr;

			BuildSet data(device);
			device->StartPublicCommandSet(data.startCommandThisFunc);

			auto dx12Holder = std::make_unique<JHdDx12CullingResultHolder>(desc.target, desc.currFrameIndex);
			dx12Holder->Build(device, desc.capacity); 
			dx12Holder->SutffClearValue(data.cmdList, data.upload.Get());

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JDx12CullingInfo>(J_CULLING_TYPE::HD_OCCLUSION, Constants::gNumFrameResources, std::move(dx12Holder), this);
			auto user = PostCreation(std::move(ownerPtr), J_CULLING_TYPE::HD_OCCLUSION);
			if (user != nullptr)
			{
				if (device->GetDeviceType() == J_GRAPHIC_DEVICE_TYPE::DX12)
				{
					Microsoft::WRL::ComPtr<ID3D12QueryHeap> newHeap;
					BuildOccQueryHeaps(device, desc.capacity, newHeap);
					occQueryHeap.push_back(std::move(newHeap)); 
				}
			}
			
			device->EndPublicCommandSet(data.startCommandThisFunc);
			if (user != nullptr && !data.startCommandThisFunc)
				device->ReStartPublicCommandSet();
			return user;
		} 
		void JDx12CullingManager::PostDestruction(const J_CULLING_TYPE type, const uint index)
		{
			if (type == J_CULLING_TYPE::HD_OCCLUSION)
				occQueryHeap.erase(occQueryHeap.begin() + index);
		}
		bool JDx12CullingManager::TryStreamOutCullingBuffer(JCullingInfo* info, const std::string& logName)
		{
#ifdef DEVELOP
			if constexpr (!allowDebug)
				return false; 

			if (info == nullptr)
				return false;
			static int count = 0;
			if (!Develop::JDevelopDebug::IsActivate())
			{
				count = 1;
				return false;
			}
			else if (count == 0)
				return false;

			--count;
			Develop::JDevelopDebug::CreatePublicLogHandler(logName);
			Develop::JDevelopDebug::PushDefaultLogHandler(logName);
			Develop::JDevelopDebug::PushLog("Result buffer");
			const uint elemtCount = info->GetResultBufferElementCount();
			for (uint i = 0; i < elemtCount; ++i)
			{
				uint r = info->IsCulled(i);
				Develop::JDevelopDebug::PushLog("index: " + std::to_string(i) +" result: " + std::to_string(r));
			}
			Develop::JDevelopDebug::Write();
			Develop::JDevelopDebug::PopDefaultLogHandler(logName);
			Develop::JDevelopDebug::DestroyPublicLogHandler(logName);
			return true;
#else
			return false;
#endif
		}
		void JDx12CullingManager::ClearResource()
		{
			occQueryHeap.clear(); 
		}
		void JDx12CullingManager::RegisterTypeData()
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
				JDx12CullingInfo* movedInfo = static_cast<JDx12CullingInfo*>(movedPtr);
				JDx12CullingManager* manager = movedInfo->manager;

				auto& vec = manager->GetCullinginfoRefVec(movedInfo->GetCullingType());

				//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
				//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
				//Release() 한다음 Reset()을 호출해야한다.
				//2024-08-15 수정 포인터만 변경하는 Swap 사용
				vec[movedInfo->GetArrayIndex()].Swap(movedInfo);
			};
			auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
			std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

			desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			JDx12CullingInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
		}
		 
		JDx12CullingResourceComputeSet::JDx12CullingResourceComputeSet(JDx12CullingManager* cm, JCullingInfo* info)
			:cm(cm), 
			info(info),
			cHolder(info != nullptr ? cm->GetDxHolder(info->GetCullingType(), info->GetArrayIndex()) : nullptr),
			gHolder(cHolder != nullptr ? cHolder->GetHolder() : nullptr),
			resource(cHolder != nullptr ? cHolder->GetResource() : nullptr)
		{}
		JDx12CullingResourceComputeSet::JDx12CullingResourceComputeSet(JDx12CullingManager* cm, const JUserPtr<JCullingInfo>& info)
			: cm(cm), 
			info(info.Get()), 
			cHolder(info != nullptr ? cm->GetDxHolder(info->GetCullingType(), info->GetArrayIndex()) : nullptr),
			gHolder(cHolder != nullptr ? cHolder->GetHolder() : nullptr),
			resource(cHolder != nullptr ? cHolder->GetResource() : nullptr)
		{}
		JDx12CullingResourceComputeSet::JDx12CullingResourceComputeSet(JDx12CullingManager* cm, JCullingInterface* cInterface, const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget)
			: cm(cm), 
			info(cm->GetCullingInfo(cType, cInterface->GetArrayIndex(cType, cTarget)).Get()),
			cHolder(info != nullptr ? cm->GetDxHolder(info->GetCullingType(), info->GetArrayIndex()) : nullptr),
			gHolder(cHolder != nullptr ? cHolder->GetHolder() : nullptr),
			resource(cHolder != nullptr ? cHolder->GetResource() : nullptr)
		{}
		bool JDx12CullingResourceComputeSet::IsValid()const noexcept
		{
			return info != nullptr;
		}
	}
}