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


#include"JCullingResultHolder.h"
#include"JCullingConstants.h"

namespace JinEngine::Graphic
{ 
	namespace Private
	{ 
	}

	uint JCullingResultHolder::GetBufferSize()const noexcept
	{
		return GetElementSize() * GetElementCount();
	}

	/*
	*  Culling Manager�� CullingInfo�� �����Ҷ� ���� �����Ǹ�(CullingInfo private has)
	*  cullingResult�׻� ��ȿ�� ���� ������
	*  RAII�� ������
	*/
	JFrustumCullingResultHolder::JFrustumCullingResultHolder(const J_CULLING_TARGET target)
		:target(target)
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
	uint JFrustumCullingResultHolder::GetElementSize()const noexcept
	{
		return sizeof(bool);
	}
	uint JFrustumCullingResultHolder::GetElementCount()const noexcept
	{
		return (uint)capacity;
	}
	J_GRAPHIC_BUFFER_TYPE JFrustumCullingResultHolder::GetBufferType()const noexcept
	{
		return J_GRAPHIC_BUFFER_TYPE::CPU;
	}
	J_CULLING_TARGET JFrustumCullingResultHolder::GetCullingTarget()const noexcept
	{
		return target;
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