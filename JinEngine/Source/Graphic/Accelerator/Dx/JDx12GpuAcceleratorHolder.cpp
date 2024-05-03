#include"JDx12GpuAcceleratorHolder.h"
#include"../../GraphicResource/JGraphicResourceInfo.h"
namespace JinEngine::Graphic
{ 
	JTlasHolder::JTlasHolder(JDx12GraphicBuffer&& buffer)
		: accelerationStructure(std::move(buffer))
	{}

	JBlasHolder::JBlasHolder(uint index)
		: index(index)
	{}
	JBlasHolder::JBlasHolder(uint index, std::vector<JDx12GraphicBuffer>&& buffer)
		: index(index), buffer(std::move(buffer))
	{}
	 
	JInstanceCache::JInstanceCache(const JUserPtr<JComponent>& comp, const uint index, const uint descStIndex)
		: comp(comp), index(index), descStIndex(descStIndex)
	{

	}

	JDx12GpuAcceleratorHolder::JDx12GpuAcceleratorHolder(std::unique_ptr<JTlasHolder>&& tlas,
		std::vector<std::unique_ptr<JBlasHolder>>&& blasVec,
		std::unordered_map<size_t, JBlasHolder*>&& blasMap,
		JDx12GraphicBufferT<D3D12_RAYTRACING_INSTANCE_DESC>&& instanceData,
		std::vector<D3D12_RAYTRACING_INSTANCE_DESC>&& instanceDescVec,
		std::vector<std::unique_ptr<JInstanceCache>>&& instanceCacheVec,
		std::unordered_map<size_t, JInstanceCache*>&& instanceCacheMap)
		: tlas(std::move(tlas)), 
		blasVec(std::move(blasVec)),
		blasMap(std::move(blasMap)), 
		instanceData(std::move(instanceData)),
		instanceDescVec(std::move(instanceDescVec)),
		instanceCacheVec(std::move(instanceCacheVec)),
		instanceCacheMap(std::move(instanceCacheMap))
	{}
	JDx12GpuAcceleratorHolder::~JDx12GpuAcceleratorHolder()
	{
		ClearResource(); 	
	}
	void JDx12GpuAcceleratorHolder::Clear()noexcept
	{
		ClearResource();
	}
	void JDx12GpuAcceleratorHolder::ClearResource()noexcept
	{
		instanceData.Clear();
		instanceDescVec.clear();
		instanceCacheVec.clear();
		instanceCacheMap.clear();
		blasMap.clear();
		blasVec.clear();
		tlas = nullptr;
	}
 	J_GRAPHIC_DEVICE_TYPE JDx12GpuAcceleratorHolder::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}	 
	JTlasHolder* JDx12GpuAcceleratorHolder::GetTlas()const noexcept
	{
		return tlas.get();
	}
	D3D12_GPU_VIRTUAL_ADDRESS JDx12GpuAcceleratorHolder::GetTlasGpuAddress()const noexcept
	{
		return tlas->accelerationStructure.GetResource()->GetGPUVirtualAddress();
	}
	uint JDx12GpuAcceleratorHolder::GetBlasCount()const noexcept
	{ 
		return (uint)blasVec.size();
	}
	JBlasHolder* JDx12GpuAcceleratorHolder::GetBlas(const size_t key)const noexcept
	{
		auto data = blasMap.find(key);
		return data != blasMap.end() ? data->second : nullptr;
	}  
	bool JDx12GpuAcceleratorHolder::HasData()const noexcept
	{
		return tlas != nullptr && blasVec.size() > 0;
	}
	void JDx12GpuAcceleratorHolder::AddBlas(std::unique_ptr<JBlasHolder>&& newBlas, const size_t key)
	{
		blasMap.emplace(key, newBlas.get());
		blasVec.push_back(std::move(newBlas));
	}
	void JDx12GpuAcceleratorHolder::RemoveBlas(const uint index, const size_t key)
	{
		blasMap.erase(key);
		const uint count = (uint)blasVec.size(); 
		for (uint i = index + 1; i < count; ++i)
			--blasVec[i]->index;  
		blasVec.erase(blasVec.begin() + index);
	}
	void JDx12GpuAcceleratorHolder::Swap(std::unique_ptr<JTlasHolder>&& newTlas)
	{
		tlas = std::move(newTlas);
	} 
}