#include"JDx12CullingResourceHolder.h"
#include"../JCullingConstants.h"
#include"../../../Core/Log/JLogMacro.h"
namespace JinEngine::Graphic
{ 
	/*
	*  Culling Manager가 CullingInfo를 생성할때 같이 생성되며(CullingInfo private has)
	*  cullingResult항상 유효한 값을 가지고
	*  RAII를 따른다
	*/
	bool JDx12CullingResourceHolder::IsGpuResource()const noexcept
	{
		return true;
	}
	bool JDx12CullingResourceHolder::CanSetValue()const noexcept
	{
		return false;
	} 

	J_GRAPHIC_BUFFER_TYPE JDx12ReadBackResourceInterface::GetBufferType()const noexcept
	{
		return J_GRAPHIC_BUFFER_TYPE::READ_BACK;
	}

	JDx12FrustumCullingResultHolder::JDx12FrustumCullingResultHolder(const J_CULLING_TARGET target, const uint frameInedx)
		:target(target)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i] = JDx12GraphicBufferT<ResultType>(L"FrustumReadBack" + std::to_wstring(i), J_GRAPHIC_BUFFER_TYPE::READ_BACK);
		current = &readBack[frameInedx];
	}
	JDx12FrustumCullingResultHolder::~JDx12FrustumCullingResultHolder()
	{
		Clear();
	}
	void JDx12FrustumCullingResultHolder::Culling(const uint index, const bool value)noexcept
	{
		OutputDebugStringA(JErrorMessage(JDx12FrustumCullingResultHolderSetCullingError).c_str());
		return;
	}
	void JDx12FrustumCullingResultHolder::CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept
	{
		memcpy(&cpuBuffer[stIndex], &current->GetCpuPointer()[stIndex], count * sizeof(ResultType));
	}
	uint JDx12FrustumCullingResultHolder::GetElementSize()const noexcept
	{
		return sizeof(ResultType);
	}
	uint JDx12FrustumCullingResultHolder::GetElementCount()const noexcept
	{
		return current == nullptr ? 0 : current->GetElementCount();
	}
	J_CULLING_TARGET JDx12FrustumCullingResultHolder::GetCullingTarget()const noexcept
	{
		return target;
	}
	ID3D12Resource* JDx12FrustumCullingResultHolder::GetResource()const noexcept
	{
		return current->GetResource();
	}  
	JDx12GraphicResourceHolder* JDx12FrustumCullingResultHolder::GetHolder()const noexcept
	{
		return current->GetHolder();
	}
	bool JDx12FrustumCullingResultHolder::IsCulled(const uint index)const noexcept
	{ 
		//0 is cull
		//1 is not cull
		//return current->GetElementCount() > index ? !current->GetCpuPointer()[index] : false;
		return current->GetElementCount() > index ? !cpuBuffer[index] : false;
	}
	void JDx12FrustumCullingResultHolder::SetCurrent(const uint frameIndex)
	{
		if (frameIndex >= Constants::gNumFrameResources)
			return;

		current = &readBack[frameIndex];
	}
	void JDx12FrustumCullingResultHolder::Build(JGraphicDevice* device, const uint newCapacity)
	{
		Clear();
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Build(device, newCapacity);
		cpuBuffer = new ResultType[newCapacity]();
		//memset(result.GetCpuPointer(), Constants::hdNonCullingValue, sizeof(uint64) * newCapacity);
	}
	void JDx12FrustumCullingResultHolder::Clear()
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Clear();
		delete[] cpuBuffer;
		cpuBuffer = nullptr;
	}
	void JDx12FrustumCullingResultHolder::SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].SutffClearValue(cmdList, uploadBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COMMON, 0);
		memset(cpuBuffer, 0, readBack[0].GetElementCount() * sizeof(ResultType));
	}

	JHzbDx12CullingResultHolder::JHzbDx12CullingResultHolder(const J_CULLING_TARGET target, const uint frameInedx)
		:target(target)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i] = JDx12GraphicBufferT<ResultType>(L"HzbOccReadBack" + std::to_wstring(i), J_GRAPHIC_BUFFER_TYPE::READ_BACK);
		current = &readBack[frameInedx];
	}
	JHzbDx12CullingResultHolder::~JHzbDx12CullingResultHolder()
	{
		Clear();
	}
	void JHzbDx12CullingResultHolder::Culling(const uint index, const bool value)noexcept
	{   
		OutputDebugStringA(JErrorMessage(JHzbDx12CullingResultHolderSetCullingValue).c_str());
		return;
	}
	void JHzbDx12CullingResultHolder::CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept
	{
		memcpy(&cpuBuffer[stIndex], &current->GetCpuPointer()[stIndex], count * sizeof(ResultType));
	}
	uint JHzbDx12CullingResultHolder::GetElementSize()const noexcept
	{
		return sizeof(ResultType);
	}
	uint JHzbDx12CullingResultHolder::GetElementCount()const noexcept
	{
		return current == nullptr ? 0 : current->GetElementCount();
	}
	J_CULLING_TARGET JHzbDx12CullingResultHolder::GetCullingTarget()const noexcept
	{
		return target;
	}
	ID3D12Resource* JHzbDx12CullingResultHolder::GetResource()const noexcept
	{
		return current->GetResource();
	}
	JDx12GraphicResourceHolder* JHzbDx12CullingResultHolder::GetHolder()const noexcept
	{
		return current->GetHolder();
	}
	void JHzbDx12CullingResultHolder::SetCurrent(const uint frameIndex)
	{
		if (frameIndex >= Constants::gNumFrameResources)
			return;

		current = &readBack[frameIndex];
	}
	bool JHzbDx12CullingResultHolder::IsCulled(const uint index)const noexcept
	{
		//0 is not cull
		//1 is cull
		return current->GetElementCount() > index ? cpuBuffer[index] : false;
		//return current->GetElementCount() > index ? current->GetCpuPointer()[index] : false;
	}
	void JHzbDx12CullingResultHolder::Build(JGraphicDevice* device, const uint newCapacity)
	{
		Clear();
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Build(device, newCapacity);
		cpuBuffer = new ResultType[newCapacity]();
	}
	void JHzbDx12CullingResultHolder::Clear()
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Clear();
		delete[] cpuBuffer;
		cpuBuffer = nullptr;
	}
	void JHzbDx12CullingResultHolder::SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].SutffClearValue(cmdList, uploadBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COMMON, 0);
		memset(cpuBuffer, 0, readBack[0].GetElementCount() * sizeof(ResultType));
	}

	JHdDx12CullingResultHolder::JHdDx12CullingResultHolder(const J_CULLING_TARGET target, const uint frameIndex)
		:target(target)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i] = JDx12GraphicBufferT<ResultType>(L"HdOccReadBack" + std::to_wstring(i), J_GRAPHIC_BUFFER_TYPE::READ_BACK);
		current = &readBack[frameIndex]; 
	}
	JHdDx12CullingResultHolder::~JHdDx12CullingResultHolder()
	{
		Clear();
	}
	void JHdDx12CullingResultHolder::Culling(const uint index, const bool value)noexcept
	{
		OutputDebugStringA(JErrorMessage(JHdDx12CullingResultHolderSetCullingValue).c_str());
		return;
	}
	void JHdDx12CullingResultHolder::CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept
	{   
		memcpy(&cpuBuffer[stIndex], &current->GetCpuPointer()[stIndex], count * sizeof(ResultType));
	}
	uint JHdDx12CullingResultHolder::GetElementSize()const noexcept
	{
		return sizeof(ResultType);
	}
	uint JHdDx12CullingResultHolder::GetElementCount()const noexcept
	{
		return current == nullptr ? 0 : current->GetElementCount();
	}
	J_CULLING_TARGET JHdDx12CullingResultHolder::GetCullingTarget()const noexcept
	{
		return target;
	}
	ID3D12Resource* JHdDx12CullingResultHolder::GetResource()const noexcept
	{
		return current->GetResource();
	} 
	JDx12GraphicResourceHolder* JHdDx12CullingResultHolder::GetHolder()const noexcept
	{
		return current->GetHolder();
	}
	void JHdDx12CullingResultHolder::SetCurrent(const uint frameIndex)
	{
		if (frameIndex >= Constants::gNumFrameResources)
			return; 

		current = &readBack[frameIndex];
	}
	bool JHdDx12CullingResultHolder::IsCulled(const uint index)const noexcept
	{
		//0 is cull
		//1 is not cull
		return current->GetElementCount() > index ? !cpuBuffer[index] : false;
	}
	void JHdDx12CullingResultHolder::Build(JGraphicDevice* device, const uint newCapacity)
	{
		Clear();  
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Build(device, newCapacity);
		cpuBuffer = new ResultType[newCapacity](); 
		//memset(result.GetCpuPointer(), Constants::hdNonCullingValue, sizeof(uint64) * newCapacity);
	}
	void JHdDx12CullingResultHolder::Clear()
	{ 
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].Clear();
		delete[] cpuBuffer;
		cpuBuffer = nullptr;
	}
	void JHdDx12CullingResultHolder::SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			readBack[i].SutffClearValue(cmdList, uploadBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COMMON, 1);
		memset(cpuBuffer, 1, readBack[0].GetElementCount() * sizeof(ResultType));
	}
}