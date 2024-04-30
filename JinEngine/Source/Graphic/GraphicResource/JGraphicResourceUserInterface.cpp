#include"JGraphicResourceUserInterface.h"
#include"JGraphicResourceHandleInterface.h"
#include"../../../Lib/DirectX/Tk/Src/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicResourceUserInterface::CallGetGraphicResourceType(JGraphicResourceHandleUserInterface& gI)const noexcept
		{
			return gI.GetGraphicResourceType();
		}
		uint JGraphicResourceUserInterface::CallGetResourceWidth(JGraphicResourceHandleUserInterface& gI)const noexcept
		{
			return gI.GetResourceWidth();
		}
		uint JGraphicResourceUserInterface::CallGetResourceHeight(JGraphicResourceHandleUserInterface& gI)const noexcept
		{
			return gI.GetResourceHeight();
		}
		uint JGraphicResourceUserInterface::CallGetResourceArrayIndex(JGraphicResourceHandleUserInterface& gI)const noexcept
		{
			return gI.GetResourceArrayIndex();
		}
		int JGraphicResourceUserInterface::CallGetHeapIndexStart(JGraphicResourceHandleUserInterface& gI, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return gI.GetHeapIndexStart(bType);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceUserInterface::CallGetGpuSrvHandle(JGraphicResourceHandleUserInterface& gI, 
			const J_GRAPHIC_BIND_TYPE bType,
			const uint index) const noexcept
		{
			return gI.GetGpuHandle(bType, index);
		}
	}
}