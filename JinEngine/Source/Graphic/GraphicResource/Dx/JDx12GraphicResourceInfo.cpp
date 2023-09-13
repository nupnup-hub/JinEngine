#include"JDx12GraphicResourceInfo.h"
#include"JDx12GraphicResourceHolder.h"  

namespace JinEngine::Graphic
{
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
	ResourceHandle JDx12GraphicResourceInfo::GetResourceHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex)const noexcept
	{  
		if (GetViewCount(bindType) <= bIndex)
			return nullptr;

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = getHandlePtr(manager, bindType, GetHeapIndexStart(bindType) + bIndex);
		//cast uint64 to void*
		//void*�� uint64�ּҰ� �ƴ϶� ������ ä������.
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
	JDx12GraphicResourceInfo::JDx12GraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, 
		JDx12GraphicResourceManager* manager,
		std::unique_ptr<JDx12GraphicResourceHolder>&& resourceHolder,
		GetHandlePtr getHandlePtr)
		:JGraphicResourceInfo(graphicResourceType),
		manager(manager), 
		resourceHolder(std::move(resourceHolder)),
		getHandlePtr(getHandlePtr)
	{}
	JDx12GraphicResourceInfo::~JDx12GraphicResourceInfo()
	{ 
		resourceHolder.reset();
	}
}