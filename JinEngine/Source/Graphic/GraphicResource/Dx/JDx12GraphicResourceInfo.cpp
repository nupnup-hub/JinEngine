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


#include"JDx12GraphicResourceInfo.h"
#include"JDx12GraphicResourceHolder.h"     
namespace JinEngine::Graphic
{
	namespace Private
	{
	}

	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceInfo::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	uint JDx12GraphicResourceInfo::GetWidth()const noexcept
	{
		return resourceHolder->GetWidth();
	}
	uint JDx12GraphicResourceInfo::GetHeight()const noexcept
	{
		return resourceHolder->GetHeight();
	}
	uint JDx12GraphicResourceInfo::GetElementCount()const noexcept
	{
		return resourceHolder->GetElementCount();
	}
	uint JDx12GraphicResourceInfo::GetElementSize()const noexcept
	{
		return resourceHolder->GetElementSize();
	}
	uint JDx12GraphicResourceInfo::GetMipmapCount()const noexcept
	{
		return resourceHolder->GetResource()->GetDesc().MipLevels;
	}
	J_GRAPHIC_RESOURCE_FORMAT JDx12GraphicResourceInfo::GetFormat()const noexcept
	{
		return resourceHolder->GetEngineFormat();
	}
	JVector2<uint>JDx12GraphicResourceInfo::GetResourceSize()const noexcept
	{
		return resourceHolder->GetResourceSize();
	}
	ResourceHandle JDx12GraphicResourceInfo::GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex)const noexcept
	{
		if (GetViewCount(bindType) <= bIndex)
			return nullptr;

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = getGpuHandlePtr(manager, bindType, GetHeapIndexStart(bindType) + bIndex);
		//cast uint64 to void*
		//void*는 uint64주소가 아니라 값으로 채워진다.
		return (ResourceHandle)handle.ptr;
	}
	ResourceHandle JDx12GraphicResourceInfo::GetResourceOptionGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex)const noexcept
	{
		if (GetOptionViewCount(bindType, opType) <= bIndex)
			return nullptr;

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = getGpuHandlePtr(manager, bindType, GetOptionHeapIndexStart(bindType, opType) + bIndex);
		//cast uint64 to void*
		//void*는 uint64주소가 아니라 값으로 채워진다.
		return (ResourceHandle)handle.ptr;
	}
	void JDx12GraphicResourceInfo::SetPrivateName(const std::wstring& name)noexcept
	{
		if (JGraphicDeviceType::IsDxType(resourceHolder->GetDeviceType()))
		{
			std::wstring fName = name == L"" ? Core::GetWName(GetGraphicResourceType()) : (Core::GetWName(GetGraphicResourceType()) + L": " + name);
			static_cast<JDxGraphicResourceHolderInterface*>(resourceHolder.get())->SetPrivateName(fName);
		}
	}
	void JDx12GraphicResourceInfo::SetPrivateOptionName(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const std::wstring& name)noexcept
	{
		if (!HasOption(opType))
			return;

		optionHolderSet->holder[(uint)opType]->GetResource()->SetName(name.c_str());
	}
	/*
	void JDx12GraphicResourceInfo::SetPostProcessLog(const uint64 frame)noexcept
	{
		if (!HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING))
			return;

		if (optionHolderSet->postProcessLog.lastUpdatedFrame != frame)
			optionHolderSet->postProcessLog.isLastUpdatedPostProcessResource = false;

		optionHolderSet->postProcessLog.lastUpdatedFrame = frame;
	}
	*/
	void JDx12GraphicResourceInfo::SetOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, std::unique_ptr<JDx12GraphicResourceHolder> optionHolder)
	{
		if (optionHolder == nullptr || HasOption(opType))
			return;

		TryCreateOptionViewInfo();
		if (optionHolderSet == nullptr)
			optionHolderSet = std::make_unique< JDx12GraphicResourceInfo::OptionHolderSet>();

		optionHolderSet->holder[(uint)opType] = std::move(optionHolder);
	}
	bool JDx12GraphicResourceInfo::HasOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept
	{
		return optionHolderSet != nullptr && optionHolderSet->holder[(uint)opType] != nullptr;
	}
	JDx12GraphicResourceInfo::JDx12GraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType,
		JDx12GraphicResourceManager* manager,
		std::unique_ptr<JDx12GraphicResourceHolder>&& resourceHolder, 
		GetGpuHandlePtr getGpuHandlePtr)
		:JGraphicResourceInfo(graphicResourceType),
		manager(manager),
		resourceHolder(std::move(resourceHolder)), 
		getGpuHandlePtr(getGpuHandlePtr)
	{
	}
	JDx12GraphicResourceInfo::~JDx12GraphicResourceInfo()
	{
		resourceHolder = nullptr;
		if (optionHolderSet != nullptr)
		{
			for (uint i = 0; i < SIZE_OF_ARRAY(optionHolderSet->holder); ++i)
				optionHolderSet->holder[i] = nullptr;		 
			optionHolderSet = nullptr;
		}
	}
}