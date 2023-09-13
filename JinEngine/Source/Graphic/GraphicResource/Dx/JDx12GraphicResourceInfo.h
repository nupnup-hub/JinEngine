#pragma once
#include"../JGraphicResourceInfo.h"
#include<d3dx12.h> 

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicResourceManager;
		class JDx12GraphicResourceHolder;
		class JDx12GraphicResourceInfo final: public JGraphicResourceInfo
		{
			REGISTER_CLASS_USE_ALLOCATOR(JDx12GraphicResourceInfo)
		public:
			using GetHandlePtr = CD3DX12_GPU_DESCRIPTOR_HANDLE(*)(JDx12GraphicResourceManager*, const J_GRAPHIC_BIND_TYPE, const uint index);
		private:
			friend class JDx12GraphicResourceManager;
		private: 
			JDx12GraphicResourceManager* manager = nullptr;
			GetHandlePtr getHandlePtr = nullptr;
		private:
			std::unique_ptr<JDx12GraphicResourceHolder> resourceHolder;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			uint GetWidth()const noexcept final;
			uint GetHeight()const noexcept final;
			ResourceHandle GetResourceHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex = 0)const noexcept final;
		public:
			//Graphic resource type name + L": " + name 
			void SetPrivateName(const std::wstring& name = L"")noexcept final; 
		public:
			JDx12GraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, 
				JDx12GraphicResourceManager* manager,
				std::unique_ptr<JDx12GraphicResourceHolder>&& resourceHolder,
				GetHandlePtr getHandlePtr);
			~JDx12GraphicResourceInfo();
		};
	}
}