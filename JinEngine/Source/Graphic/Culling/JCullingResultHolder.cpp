#include"JCullingResultHolder.h"
#include"JCullingConstants.h"

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
	JFrustumCullingResultHolder::JFrustumCullingResultHolder()
	{}
	JFrustumCullingResultHolder::~JFrustumCullingResultHolder()
	{
		Clear();
	}
	void JFrustumCullingResultHolder::Culling(const uint index, const bool value)noexcept
	{		
		if(capacity > index)
			cullingResult[index] = value;
	}
	uint JFrustumCullingResultHolder::GetBuffSize()const noexcept
	{
		return (uint)capacity;
	}
	bool JFrustumCullingResultHolder::IsCulled(const uint index)const noexcept
	{
		return capacity > index ? cullingResult[index] : false;
	}
	bool JFrustumCullingResultHolder::IsGpuResource()const noexcept
	{
		return false;
	}
	bool JFrustumCullingResultHolder::CanSetValue()const noexcept
	{
		return true;
	}
	void JFrustumCullingResultHolder::Build(const uint newCapacity)
	{
		if (cullingResult != nullptr)
		{
			bool* preVec = cullingResult;
			bool* newVec = new bool[newCapacity]();
			uint copyEnd = capacity > newCapacity ? newCapacity : (uint)capacity;
			for (uint i = 0; i < copyEnd; ++i)
					newVec[i] = preVec[i];

			cullingResult = newVec;
			delete[] preVec;
		}
		else
			cullingResult = new bool[newCapacity]();
		capacity = newCapacity;
	}
	void JFrustumCullingResultHolder::Clear()
	{
		if (cullingResult != nullptr)
			delete[] cullingResult;

		cullingResult = nullptr;
		capacity = 0;
	} 
}