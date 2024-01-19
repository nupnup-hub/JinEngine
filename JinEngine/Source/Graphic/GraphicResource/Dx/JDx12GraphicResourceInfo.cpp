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
	uint JDx12GraphicResourceInfo::GetMipmapCount()const noexcept
	{
		return resourceHolder->GetResource()->GetDesc().MipLevels;
	}
	ResourceHandle JDx12GraphicResourceInfo::GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex)const noexcept
	{  
		if (GetViewCount(bindType) <= bIndex)
			return nullptr;
		 
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = getHandlePtr(manager, bindType, GetHeapIndexStart(bindType) + bIndex);
		//cast uint64 to void*
		//void*는 uint64주소가 아니라 값으로 채워진다.
		return (ResourceHandle)handle.ptr;
	}
	ResourceHandle JDx12GraphicResourceInfo::GetResourceOptionGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex)const noexcept
	{
		if (GetOptionViewCount(bindType, opType) <= bIndex)
			return nullptr;

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = getHandlePtr(manager, bindType, GetOptionHeapIndexStart(bindType, opType) + bIndex);
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
		GetHandlePtr getHandlePtr)
		:JGraphicResourceInfo(graphicResourceType),
		manager(manager), 
		resourceHolder(std::move(resourceHolder)),
		getHandlePtr(getHandlePtr)
	{
	}
	JDx12GraphicResourceInfo::~JDx12GraphicResourceInfo()
	{
		resourceHolder = nullptr;
		if (optionHolderSet != nullptr)
		{
			for (auto& data : optionHolderSet->holder)
				data = nullptr;
			optionHolderSet = nullptr;
		}
	}
}