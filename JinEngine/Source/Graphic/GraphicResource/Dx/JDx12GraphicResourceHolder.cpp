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


#include"JDx12GraphicResourceHolder.h" 
#include"JDx12GraphicResourceFormat.h" 

namespace JinEngine::Graphic
{ 
	//static size_t totalAllocatedByte = 0;
//	static int allocatedCount = 0;

	JDx12GraphicResourceHolderDesc::JDx12GraphicResourceHolderDesc(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES state)
		:resource(std::move(newResource)), state(state)
	{ 
		JDx12GraphicResourceHolderDesc::elementSize = JDx12Format::DetermineElementSize(resource->GetDesc().Format);
	}
	JDx12GraphicResourceHolderDesc::JDx12GraphicResourceHolderDesc(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES state, const uint elementSize)
		:resource(std::move(newResource)), state(state)
	{
		const D3D12_RESOURCE_DESC desc = resource->GetDesc();
		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			JDx12GraphicResourceHolderDesc::elementSize = elementSize;
		else
			JDx12GraphicResourceHolderDesc::elementSize = JDx12Format::DetermineElementSize(desc.Format);
	}
	bool JDx12GraphicResourceHolderDesc::IsValid()const noexcept
	{
		return resource != nullptr;
	}

 	JDx12GraphicResourceHolder::JDx12GraphicResourceHolder(JDx12GraphicResourceHolderDesc&& desc)
		:resource(std::move(desc.resource)), state(desc.state), defaultState(desc.state), elementSize(desc.elementSize)
	{
	}
	JDx12GraphicResourceHolder::~JDx12GraphicResourceHolder()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceHolder::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	uint JDx12GraphicResourceHolder::GetWidth()const noexcept
	{ 
		return resource->GetDesc().Width;
	}
	uint JDx12GraphicResourceHolder::GetHeight()const noexcept
	{
		return resource->GetDesc().Height;
	}
	uint JDx12GraphicResourceHolder::GetArrayCount()const noexcept
	{
		return resource->GetDesc().DepthOrArraySize;
	}
	uint JDx12GraphicResourceHolder::GetElementCount()const noexcept
	{
		auto desc = resource->GetDesc();
		//byte buffer
		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && desc.Format == DXGI_FORMAT_UNKNOWN)
			return (GetWidth() * GetHeight()) / elementSize;
		else
			return GetWidth() * GetHeight();
	}
	uint JDx12GraphicResourceHolder::GetElementSize()const noexcept
	{
		return elementSize;
	}
	JVector2<uint> JDx12GraphicResourceHolder::GetResourceSize()const noexcept
	{
		auto desc = resource->GetDesc();
		return JVector2<uint>(desc.Width, desc.Height);
	}
	J_GRAPHIC_RESOURCE_FORMAT JDx12GraphicResourceHolder::GetEngineFormat()const noexcept
	{ 
		return JDx12Format::ConvertEngine(resource->GetDesc().Format);
	}
	ID3D12Resource* JDx12GraphicResourceHolder::GetResource() const noexcept
	{
		return resource.Get();
	}
	D3D12_RESOURCE_STATES JDx12GraphicResourceHolder::GetState()const noexcept
	{
		return state;
	}
	D3D12_RESOURCE_STATES JDx12GraphicResourceHolder::GetDefaultState()const noexcept
	{
		return defaultState;
	}
	void JDx12GraphicResourceHolder::SetPrivateName(const std::wstring& name)noexcept
	{
		resource->SetName(name.c_str());
	}
	void JDx12GraphicResourceHolder::SetState(const D3D12_RESOURCE_STATES newState)noexcept
	{
		state = newState;
	} 
	bool JDx12GraphicResourceHolder::IsBuffer()const noexcept
	{ 
		return resource->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
	}
	bool JDx12GraphicResourceHolder::HasValidResource()const noexcept
	{
		return resource != nullptr;
	}
	void JDx12GraphicResourceHolder::Swap(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES newState)noexcept
	{
		resource = std::move(newResource); 
		defaultState = newState;
		state = newState;
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
		ClearResource();
	}
	void JDx12GraphicResourceHolder::ClearResource()
	{
		if (resource == nullptr)
			return;
		 
		D3D12_HEAP_PROPERTIES pHeapProperties;
		D3D12_HEAP_FLAGS pHeapFlags;
		resource->GetHeapProperties(&pHeapProperties, &pHeapFlags);
		 
		if (!Core::HasSQValueEnum(pHeapFlags, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_SHARED))
		{
			resource = nullptr;
			/*
			D3D12_RESOURCE_DESC desc = resource->GetDesc();
			if (pHeapProperties.Type == D3D12_HEAP_TYPE_UPLOAD)
				resource = nullptr;
				//resource.Reset();
			else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
				resource = nullptr;
				//resource.Reset();
			else
			{
				resource = nullptr;
				//resource->Release();
				//resource.Reset();
			}
			*/
		} 
	}
}