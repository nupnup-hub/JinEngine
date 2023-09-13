#include"JDx12GraphicResourceHolder.h" 
#include<memory>

namespace JinEngine::Graphic
{ 
	JDx12GraphicResourceHolder::JDx12GraphicResourceHolder(Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
		:resource(std::move(resource))
	{
	}
	JDx12GraphicResourceHolder::~JDx12GraphicResourceHolder()
	{
		Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceHolder::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	uint JDx12GraphicResourceHolder::GetWidth()const noexcept
	{
		return resource->GetDesc().Height;
	}
	uint JDx12GraphicResourceHolder::GetHeight()const noexcept
	{
		return resource->GetDesc().Width;
	}
	ID3D12Resource* JDx12GraphicResourceHolder::GetResource() const noexcept
	{
		return resource.Get();
	}
	void JDx12GraphicResourceHolder::SetPrivateName(const std::wstring& name)noexcept
	{
		resource->SetName(name.c_str());
	}
	bool JDx12GraphicResourceHolder::HasValidResource()const noexcept
	{
		return resource != nullptr;
	}
	void JDx12GraphicResourceHolder::Swap(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource)noexcept
	{
		resource = std::move(newResource); 
	}
	BYTE* JDx12GraphicResourceHolder::Map()const noexcept
	{
		BYTE* mappedData = nullptr;
		if (resource != nullptr)
			resource->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		return mappedData;
	}
	void JDx12GraphicResourceHolder::UnMap()const noexcept
	{	
		if (resource != nullptr)
			resource->Unmap(0, nullptr);
	}
	void JDx12GraphicResourceHolder::Clear()
	{
		if (resource == nullptr)
			return;

		D3D12_HEAP_PROPERTIES pHeapProperties;
		D3D12_HEAP_FLAGS pHeapFlags;
		resource->GetHeapProperties(&pHeapProperties, &pHeapFlags);
		 
		if ((((int)pHeapFlags & (int)D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_SHARED) == 0))
		{
			D3D12_RESOURCE_DESC desc = resource->GetDesc();
			if (pHeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD)
				resource.Reset();
			else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
				resource.Reset();
			else
			{ 
				resource->Release(); 
				resource.Reset();
			}	 
		}
	}
}