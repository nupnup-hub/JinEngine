#include"JDx12CullingResourceHolder.h"
#include"../JCullingConstants.h"

namespace JinEngine::Graphic
{
	namespace Private
	{
	}
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

	JHzbDx12CullingResultHolder::JHzbDx12CullingResultHolder()
		:readBack(L"HzbOccReadBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK)
	{}
	JHzbDx12CullingResultHolder::~JHzbDx12CullingResultHolder()
	{
		Clear();
	}
	void JHzbDx12CullingResultHolder::Culling(const uint index, const bool value)noexcept
	{   
		OutputDebugStringA(JErrorMessage(HzbCanSetCullingValue).c_str());
		return;
	}
	uint JHzbDx12CullingResultHolder::GetBuffSize()const noexcept
	{
		return readBack.GetElementCount();
	}
	bool JHzbDx12CullingResultHolder::IsCulled(const uint index)const noexcept
	{
		return readBack.GetElementCount() > index ? readBack.GetCpuPointer()[index] : false;
	}
	ID3D12Resource* JHzbDx12CullingResultHolder::GetResource()const noexcept
	{
		return readBack.GetResource();
	}
	void JHzbDx12CullingResultHolder::Build(JGraphicDevice* device, const uint newCapacity)
	{
		readBack.Clear();
		readBack.Build(device, newCapacity);
	}
	void JHzbDx12CullingResultHolder::Clear()
	{
		readBack.Clear();
	}

	JHdDx12CullingResultHolder::JHdDx12CullingResultHolder()
		:readBack(L"HdOccReadBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK)
	{}
	JHdDx12CullingResultHolder::~JHdDx12CullingResultHolder()
	{
		Clear();
	}
	void JHdDx12CullingResultHolder::Culling(const uint index, const bool value)noexcept
	{
		OutputDebugStringA(JErrorMessage(HdCanSetCullingValue).c_str());
		return;
	}
	uint JHdDx12CullingResultHolder::GetBuffSize()const noexcept
	{
		return readBack.GetElementCount();
	}
	ID3D12Resource* JHdDx12CullingResultHolder::GetResource()const noexcept
	{
		return readBack.GetResource();
	}
	ID3D12Resource* JHdDx12CullingResultHolder::GetPredictResource()const noexcept
	{
		return readBack.GetResource();
	}
	ID3D12Resource* JHdDx12CullingResultHolder::GetReadBackResource()const noexcept
	{
		return readBack.GetResource();
	}
	bool JHdDx12CullingResultHolder::IsCulled(const uint index)const noexcept
	{
		//0 is cull
		//1 is not cull
		return readBack.GetElementCount() > index ? !readBack.GetCpuPointer()[index] : false;
	}
	void JHdDx12CullingResultHolder::Build(JGraphicDevice* device, const uint newCapacity)
	{ 
		readBack.Clear();
		readBack.Build(device, newCapacity);
		//memset(result.GetCpuPointer(), Constants::hdNonCullingValue, sizeof(uint64) * newCapacity);
	}
	void JHdDx12CullingResultHolder::Clear()
	{
		readBack.Clear();
	}
}