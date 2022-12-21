#pragma once
#include"../../Core/JDataType.h"
#include"JGraphicResourceType.h" 

struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceHandleBaseInterface;
		class JGraphicResourceUserInterface
		{
		protected:
			Graphic::J_GRAPHIC_RESOURCE_TYPE CallGetGraphicResourceType(JGraphicResourceHandleBaseInterface& gI)const noexcept;
			uint CallGetResourceWidth(JGraphicResourceHandleBaseInterface& gI)const noexcept;
			uint CallGetResourceHeight(JGraphicResourceHandleBaseInterface& gI)const noexcept;
			//0 ~ n
			uint CallGetResourceArrayIndex(JGraphicResourceHandleBaseInterface& gI)const noexcept;
			//n(predefined resource) ~ m
			uint CallGetHeapIndexStart(JGraphicResourceHandleBaseInterface& gI, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE CallGetGpuSrvHandle(JGraphicResourceHandleBaseInterface& gI, 
				const J_GRAPHIC_BIND_TYPE bType,
				const uint index) const noexcept;
		};
	}
}