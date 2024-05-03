#include"JDx12CullingManager.h"
#include"JDx12CullingResourceHolder.h"
#include"../JCullingInfo.h"  
#include"../JCullingInterface.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
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
#if defined (GRAPIC_DEBUG) && defined(DEVELOP)
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
			case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
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
			case JinEngine::Graphic::J_CULLING_TYPE::HZB_OCCLUSION:
			{
				static_cast<JHzbDx12CullingResultHolder*>(holder)->Build(device, capacity);
				static_cast<JHzbDx12CullingResultHolder*>(holder)->SutffClearValue(data.cmdList, data.upload.Get());
				break;
			}
			case JinEngine::Graphic::J_CULLING_TYPE::HD_OCCLUSION:
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
			if (dx12Set.preInfo.frame.upPLightCapacity != dx12Set.newInfo.frame.upPLightCapacity ||
				dx12Set.preInfo.frame.upSLightCapacity != dx12Set.newInfo.frame.upSLightCapacity ||
				dx12Set.preInfo.frame.upRLightCapacity != dx12Set.newInfo.frame.upRLightCapacity)
			{
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, dx12Set.device,
					dx12Set.newInfo.frame.GetLocalLightCapacity(), J_CULLING_TARGET::LIGHT);
			}
			if (dx12Set.preInfo.frame.upBoundingObjCapacity != dx12Set.newInfo.frame.upBoundingObjCapacity)
			{ 
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, dx12Set.device, dx12Set.newInfo.frame.upBoundingObjCapacity, J_CULLING_TARGET::RENDERITEM);
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::HD_OCCLUSION, dx12Set.device, dx12Set.newInfo.frame.upBoundingObjCapacity);
			}
			if (dx12Set.preInfo.frame.upHzbObjCapacity != dx12Set.newInfo.frame.upHzbObjCapacity)
			{ 
				JCullingManager::ReBuildBuffer(J_CULLING_TYPE::HZB_OCCLUSION, dx12Set.device, dx12Set.newInfo.frame.upHzbObjCapacity);
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
				ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::FRUSTUM, Constants::gNumFrameResources, std::move(resultHolder));
			}
			else
			{
				auto resultHolder = std::make_unique<JFrustumCullingResultHolder>(desc.target);
				resultHolder->Build(desc.capacity);
				ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::FRUSTUM, Constants::gNumFrameResources, std::move(resultHolder));
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

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::HZB_OCCLUSION, Constants::gNumFrameResources, std::move(dx12Holder));
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

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::HD_OCCLUSION, Constants::gNumFrameResources, std::move(dx12Holder));
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
		void JDx12CullingManager::Clear()
		{
			ClearResource();
			JCullingManager::Clear();
		}
		void JDx12CullingManager::ClearResource()
		{
			occQueryHeap.clear(); 
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
		JDx12CullingResourceComputeSet::JDx12CullingResourceComputeSet(JDx12CullingManager* cm, const JCullingUserInterface& cInterface, const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget)
			: cm(cm), 
			info(cm->GetCullingInfo(cType, cInterface.GetArrayIndex(cType, cTarget)).Get()),
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