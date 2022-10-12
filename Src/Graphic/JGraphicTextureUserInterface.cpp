#include"JGraphicTextureUserInterface.h"
#include"JGraphicTexture.h"
#include"../../Lib/DirectX/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		uint JGraphicTextureUserInterface::CallGetTxtWidth(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtWidth();
		}
		uint JGraphicTextureUserInterface::CallGetTxtHeight(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtHeight();
		}
		uint JGraphicTextureUserInterface::CallGetTxtRtvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtRtvHeapIndex();
		}
		uint JGraphicTextureUserInterface::CallGetTxtDsvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtDsvHeapIndex();
		}
		uint JGraphicTextureUserInterface::CallGetTxtSrvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtSrvHeapIndex();
		}
		uint JGraphicTextureUserInterface::CallGetTxtVectorIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtVectorIndex();
		}
		Graphic::J_GRAPHIC_TEXTURE_TYPE JGraphicTextureUserInterface::CallGetTxtType(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtType();
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicTextureUserInterface::CallGetCpuHandle(JGraphicTextureInterface& gtI) const noexcept
		{
			return gtI.GetCpuHandle();
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicTextureUserInterface::CallGetGpuHandle(JGraphicTextureInterface& gtI) const noexcept
		{
			return gtI.GetGpuHandle();
		}
	}
}