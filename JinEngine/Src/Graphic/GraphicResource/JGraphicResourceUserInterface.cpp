#include"JGraphicResourceUserInterface.h"
#include"JGraphicResourceHandleInterface.h"
#include"../../../Lib/DirectX/Tk/Src/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicResourceUserInterface::CallGetGraphicResourceType(JGraphicResourceHandleBaseInterface& gI)const noexcept
		{
			return gI.GetGraphicResourceType();
		}
		uint JGraphicResourceUserInterface::CallGetResourceWidth(JGraphicResourceHandleBaseInterface& gI)const noexcept
		{
			return gI.GetResourceWidth();
		}
		uint JGraphicResourceUserInterface::CallGetResourceHeight(JGraphicResourceHandleBaseInterface& gI)const noexcept
		{
			return gI.GetResourceHeight();
		}
		uint JGraphicResourceUserInterface::CallGetResourceArrayIndex(JGraphicResourceHandleBaseInterface& gI)const noexcept
		{
			return gI.GetResourceArrayIndex();
		}
		uint JGraphicResourceUserInterface::CallGetHeapIndexStart(JGraphicResourceHandleBaseInterface& gI, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return gI.GetHeapIndexStart(bType);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceUserInterface::CallGetGpuSrvHandle(JGraphicResourceHandleBaseInterface& gI, 
			const J_GRAPHIC_BIND_TYPE bType,
			const uint index) const noexcept
		{
			return gI.GetGpuHandle(bType, index);
		}
	}
}