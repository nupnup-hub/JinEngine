#include"JDx12CullingManager.h"
#include"JDx12CullingResourceHolder.h"
#include"../JCullingInfo.h"  

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			//query heap count = initOccQueryHeapCapacity <= newCapacity
			static Microsoft::WRL::ComPtr<ID3D12QueryHeap> BuildOccQueryHeaps(JGraphicDevice* device, const size_t capacity)
			{
				if (device == nullptr || device->GetDeviceType() != J_GRAPHIC_DEVICE_TYPE::DX12)
					return nullptr;

				ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
				Microsoft::WRL::ComPtr<ID3D12QueryHeap> newQueryHeap;
				D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
				queryHeapDesc.Count = (uint)capacity;
				queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
				ThrowIfFailedHr(d3d12Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&newQueryHeap)));
				newQueryHeap->SetName(L"Occlusion Query Heap ");
				return std::move(newQueryHeap);
			}
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
		void JDx12CullingManager::ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index)
		{
			if (info == nullptr)
				return;

			const J_CULLING_TYPE cType = info->GetCullingType();
			switch (cType)
			{
			case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
			{
				static_cast<JFrustumCullingResultHolder*>(GetHolder(info))->Build(capacity);
				break;
			}
			case JinEngine::Graphic::J_CULLING_TYPE::HZB_OCCLUSION:
			{
				static_cast<JHzbDx12CullingResultHolder*>(GetHolder(info))->Build(device, capacity);
				break;
			}
			case JinEngine::Graphic::J_CULLING_TYPE::HD_OCCLUSION:
			{
				static_cast<JHdDx12CullingResultHolder*>(GetHolder(info))->Build(device, capacity);
				occQueryHeap[index] = BuildOccQueryHeaps(device, capacity);
				break;
			}
			default:
				break;
			}
		}
		ID3D12QueryHeap* JDx12CullingManager::GetQueryHeap(const uint index)const noexcept
		{
			return occQueryHeap.size() > index ? occQueryHeap[index].Get() : nullptr;
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateFrsutumData(const size_t capacity)
		{
			auto resultHolder = std::make_unique<JFrustumCullingResultHolder>();
			resultHolder->Build(capacity);

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::FRUSTUM, std::move(resultHolder));
			return PostCreation(std::move(ownerPtr), J_CULLING_TYPE::FRUSTUM);
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateHzbOcclusionData(JGraphicDevice* device, const size_t capacity)
		{
			std::unique_ptr<JCullingResultHolder> resultHolder;
			if (device->GetDeviceType() == J_GRAPHIC_DEVICE_TYPE::DX12)
			{
				auto dx12Holder = std::make_unique<JHzbDx12CullingResultHolder>();
				dx12Holder->Build(device, capacity);
				resultHolder = std::move(dx12Holder);
			}

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::HZB_OCCLUSION, std::move(resultHolder));
			return PostCreation(std::move(ownerPtr), J_CULLING_TYPE::HZB_OCCLUSION);
		}
		JUserPtr<JCullingInfo> JDx12CullingManager::CreateHdOcclusionData(JGraphicDevice* device, const size_t capacity)
		{
			std::unique_ptr<JCullingResultHolder> resultHolder;
			if (device->GetDeviceType() == J_GRAPHIC_DEVICE_TYPE::DX12)
			{
				auto dx12Holder = std::make_unique<JHdDx12CullingResultHolder>();
				dx12Holder->Build(device, capacity);
				resultHolder = std::move(dx12Holder);
			}

			auto ownerPtr = Core::JPtrUtil::MakeOwnerPtr<JCullingInfo>(this, J_CULLING_TYPE::HD_OCCLUSION, std::move(resultHolder));
			auto user = PostCreation(std::move(ownerPtr), J_CULLING_TYPE::HD_OCCLUSION);
			if (user != nullptr)
			{
				if (device->GetDeviceType() == J_GRAPHIC_DEVICE_TYPE::DX12)
					occQueryHeap.push_back(BuildOccQueryHeaps(device, capacity));
			}
			return user;
		}
		void JDx12CullingManager::PostDestruction(const J_CULLING_TYPE type, const uint index)
		{
			if (type == J_CULLING_TYPE::HD_OCCLUSION)
				occQueryHeap.erase(occQueryHeap.begin() + index);
		}
		void JDx12CullingManager::Clear()
		{
			occQueryHeap.clear();
			JCullingManager::Clear();
		}
	}
}