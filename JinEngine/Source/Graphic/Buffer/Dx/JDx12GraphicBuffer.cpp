#include"JDx12GraphicBuffer.h"
#include"../../../Core/Math/JMathHelper.h"
namespace JinEngine::Graphic
{
	JDx12GraphicBuffer::JDx12GraphicBuffer(const std::wstring& name, const J_GRAPHIC_BUFFER_TYPE type, const size_t elementByteSize)
		:name(name), type(type), elementByteSize(elementByteSize)
	{
		if (type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT)
			JDx12GraphicBuffer::elementByteSize = CalcConstantBufferByteSize(elementByteSize);
	}
	JDx12GraphicBuffer::~JDx12GraphicBuffer()
	{
		ClearOwnResource();
	}
	void JDx12GraphicBuffer::Build(JGraphicDevice* device, const uint newElementcount)
	{
		if (!IsSameDevice(device))
			return;

		Build(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), newElementcount);
	}
	void JDx12GraphicBuffer::Build(ID3D12Device* device, const uint newElementcount)
	{
		if (IsValid())
			Clear();
		elementcount = newElementcount;

		if (type == J_GRAPHIC_BUFFER_TYPE::READ_BACK)
			BuildReadBack(device);
		else if (type == J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS)
			BuildUav(device);
		else if (type == J_GRAPHIC_BUFFER_TYPE::OCC_PREDICT)
			BuildOccPredict(device);
		else if (type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER || type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT)
			BuildUpload(device);
		else if (type == J_GRAPHIC_BUFFER_TYPE::COMMON)
			BuildCommon(device);
		else
			return;
	}
	void JDx12GraphicBuffer::Clear()noexcept
	{
		ClearOwnResource();
		SetValid(false);
	}
	void JDx12GraphicBuffer::StuffValue(ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* uploadBuffer,
		const D3D12_RESOURCE_STATES beforeState,
		const D3D12_RESOURCE_STATES afterState,
		const void* data)
	{
		if (type != J_GRAPHIC_BUFFER_TYPE::READ_BACK && type != J_GRAPHIC_BUFFER_TYPE::COMMON)
			return;

		JDx12GraphicResourceCreation::UploadData(cmdList, holder.GetResource(), uploadBuffer, beforeState, afterState, data, elementcount, elementByteSize, 1);
	}
	void JDx12GraphicBuffer::BuildReadBack(ID3D12Device* device, D3D12_RESOURCE_STATES initState)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size_t)elementByteSize * elementcount);

		if (fixedInitState != (D3D12_RESOURCE_STATES)invalidIndex)
			initState = fixedInitState;

		ThrowIfFailedHr(device->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			initState,
			nullptr,
			IID_PPV_ARGS(&buffer)));
		canMapped = true;
		BuildHolder(std::move(buffer), initState);
	}
	void JDx12GraphicBuffer::BuildUav(ID3D12Device* device,D3D12_RESOURCE_STATES initState)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size_t)elementByteSize * elementcount, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		if (fixedInitState != (D3D12_RESOURCE_STATES)invalidIndex)
			initState = fixedInitState;

		ThrowIfFailedHr(device->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			initState,
			//초기상태설정시 D3D12_RESOURCE_STATE_UNORDERED_ACCESS은 무시되며 자동으로 D3D12_RESOURCE_STATE_COMMON으로
			//내부에서 설정되며 리소스 사용시 D3D12_RESOURCE_STATE_UNORDERED_ACCESS로 수동변환이 필요하다
			//D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			nullptr,
			IID_PPV_ARGS(&buffer)));
		BuildHolder(std::move(buffer), initState);
	}
	void JDx12GraphicBuffer::BuildOccPredict(ID3D12Device* device)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		auto queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer((size_t)elementByteSize * elementcount);
		//초기상태설정시 D3D12_RESOURCE_STATE_PREDICATION은 무시되며 자동으로 D3D12_RESOURCE_STATE_COMMON으로
		//내부에서 설정되며 리소스 사용시 D3D12_RESOURCE_STATE_PREDICATION로 수동변환이 필요하다
		//D3D12_RESOURCE_STATE_PREDICATION			 
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;
		if (fixedInitState != (D3D12_RESOURCE_STATES)invalidIndex)
			initState = fixedInitState;

		ThrowIfFailedG(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&queryResultDesc,
			initState,
			nullptr,
			IID_PPV_ARGS(&buffer)));
		BuildHolder(std::move(buffer), initState);
	}
	void JDx12GraphicBuffer::BuildUpload(ID3D12Device* device)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size_t)elementByteSize * elementcount);
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_GENERIC_READ;
		if (fixedInitState != (D3D12_RESOURCE_STATES)invalidIndex)
			initState = fixedInitState;

		ThrowIfFailedHr(device->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			initState,
			nullptr,
			IID_PPV_ARGS(&buffer)));
		canMapped = true;
		BuildHolder(std::move(buffer), initState);
	}
	void JDx12GraphicBuffer::BuildCommon(ID3D12Device* device, D3D12_RESOURCE_STATES initState)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size_t)elementByteSize * elementcount);
		if (fixedInitState != (D3D12_RESOURCE_STATES)invalidIndex)
			initState = fixedInitState;

		ThrowIfFailedHr(device->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			initState,
			nullptr,
			IID_PPV_ARGS(&buffer)));
		BuildHolder(std::move(buffer), initState);
	}
	void JDx12GraphicBuffer::BuildHolder(Microsoft::WRL::ComPtr<ID3D12Resource>&& buffer, const D3D12_RESOURCE_STATES initState)
	{
		buffer->SetName(name.c_str());
		holder.Swap(std::move(buffer), initState);
		if (canMapped)
			mappedData = holder.Map();
		SetValid(true);
	}
	void JDx12GraphicBuffer::CopyData(const uint elementIndex, const void* data)
	{
		memcpy(&mappedData[elementIndex * elementByteSize], data, elementByteSize);
	}
	void JDx12GraphicBuffer::CopyData(const uint elementIndex, const uint count, const void* data)
	{ 
		memcpy(&mappedData[elementIndex * elementByteSize], data, (size_t)elementByteSize * count);
	}
	std::wstring JDx12GraphicBuffer::GetName()const noexcept
	{
		return name;
	}
	J_GRAPHIC_BUFFER_TYPE JDx12GraphicBuffer::GetBufferType()const noexcept
	{
		return type;
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBuffer::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	uint JDx12GraphicBuffer::GetElementCount()const noexcept
	{
		return elementcount;
	}
	uint JDx12GraphicBuffer::GetElementByteSize()const noexcept
	{
		return elementByteSize;
	}
	BYTE* JDx12GraphicBuffer::GetCpuBytePointer()const noexcept
	{
		return mappedData;
	}
	void* JDx12GraphicBuffer::GetCpuPointer()const noexcept
	{
		return static_cast<void*>(mappedData);
	}
	JDx12GraphicResourceHolder* JDx12GraphicBuffer::GetHolder()noexcept
	{
		return &holder;
	}
	ID3D12Resource* JDx12GraphicBuffer::GetResource()const noexcept
	{
		return holder.GetResource();
	}
	D3D12_GPU_VIRTUAL_ADDRESS JDx12GraphicBuffer::GetGpuAddress(const uint addressOffset)noexcept
	{
		return holder.GetResource()->GetGPUVirtualAddress() + addressOffset * elementByteSize;
	} 
	BYTE JDx12GraphicBuffer::GetData(const uint index)const noexcept
	{
		return (BYTE)mappedData[index * elementByteSize];
	}
	void JDx12GraphicBuffer::SetFixedInitState(const D3D12_RESOURCE_STATES state)
	{
		fixedInitState = state;
	}
	void JDx12GraphicBuffer::SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{
		commandList->SetGraphicsRootConstantBufferView(rootIndex, GetGpuAddress(addressOffset));
	}
	void JDx12GraphicBuffer::SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{ 
		commandList->SetComputeRootConstantBufferView(rootIndex, GetGpuAddress(addressOffset));
	}
	void JDx12GraphicBuffer::SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{
		commandList->SetGraphicsRootShaderResourceView(rootIndex, GetGpuAddress(addressOffset));
	}
	void JDx12GraphicBuffer::SetComputeRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{
		commandList->SetComputeRootShaderResourceView(rootIndex, GetGpuAddress(addressOffset));
	}
	void JDx12GraphicBuffer::SetGraphicsRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{
		commandList->SetGraphicsRootUnorderedAccessView(rootIndex, GetGpuAddress(addressOffset));
	}
	void JDx12GraphicBuffer::SetComputeRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
	{
		commandList->SetComputeRootUnorderedAccessView(rootIndex, GetGpuAddress(addressOffset));
	}
	bool JDx12GraphicBuffer::CanMappedCpuPointer()const noexcept
	{
		return true;
	}
	void JDx12GraphicBuffer::ClearOwnResource()
	{
		if (canMapped && mappedData != nullptr)
			holder.UnMap();
		//if (canMapped && IsValid())
		//	holder.UnMap();
		mappedData = nullptr;
		holder.Clear();
		elementcount = 0;
	}
	uint JDx12GraphicBuffer::CalcConstantBufferByteSize(const uint byteSize)noexcept
	{
		// Constants buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}
}
 