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
			using GetCpuHandlePtr = CD3DX12_CPU_DESCRIPTOR_HANDLE(*)(JDx12GraphicResourceManager*, const J_GRAPHIC_BIND_TYPE, const uint index);
			using GetGpuHandlePtr = CD3DX12_GPU_DESCRIPTOR_HANDLE(*)(JDx12GraphicResourceManager*, const J_GRAPHIC_BIND_TYPE, const uint index);
		private:
			struct OptionHolderSet
			{
				//struct PostProcessLog
				//{
				////public:
				//	uint64 lastUpdatedFrame = 0;
				//	bool isLastUpdatedPostProcessResource = false;
				//};
			public:
				std::unique_ptr<JDx12GraphicResourceHolder> holder[(uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT];
				//PostProcessLog postProcessLog;
			};
		private:
			friend class JDx12GraphicResourceManager;
		private: 
			JDx12GraphicResourceManager* manager = nullptr; 
			GetGpuHandlePtr getGpuHandlePtr = nullptr;
		private:
			std::unique_ptr<JDx12GraphicResourceHolder> resourceHolder = nullptr;
			std::unique_ptr<OptionHolderSet> optionHolderSet = nullptr;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final; 
			uint GetWidth()const noexcept final;
			uint GetHeight()const noexcept final;
			uint GetElementCount()const noexcept final;
			uint GetElementSize()const noexcept final;
			uint GetMipmapCount()const noexcept final; 
			J_GRAPHIC_RESOURCE_FORMAT GetFormat()const noexcept final;
			JVector2<uint> GetResourceSize()const noexcept final;
			ResourceHandle GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex = 0)const noexcept final;
			ResourceHandle GetResourceOptionGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex = 0)const noexcept final;
		public:
			//Graphic resource type name + L": " + name 
			void SetPrivateName(const std::wstring& name = L"")noexcept final; 
			void SetPrivateOptionName(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const std::wstring& name)noexcept final;
		private:
			void SetOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, std::unique_ptr<JDx12GraphicResourceHolder> optionHolder);
		public:
			bool HasOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept final; 
		public:
			JDx12GraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, 
				JDx12GraphicResourceManager* manager,
				std::unique_ptr<JDx12GraphicResourceHolder>&& resourceHolder, 
				GetGpuHandlePtr getGpuHandlePtr);
			~JDx12GraphicResourceInfo();
		};
	}
}