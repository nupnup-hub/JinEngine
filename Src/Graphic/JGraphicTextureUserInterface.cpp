#include"JGraphicTextureUserInterface.h"
#include"JGraphicTexture.h"
#include"../../Lib/DirectX/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		uint JGraphicTextureUserInterface::GetTxtWidth(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtWidth();
		}
		uint JGraphicTextureUserInterface::GetTxtHeight(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtHeight();
		}
		uint JGraphicTextureUserInterface::GetTxtRtvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtRtvHeapIndex();
		}
		uint JGraphicTextureUserInterface::GetTxtDsvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtDsvHeapIndex();
		}
		uint JGraphicTextureUserInterface::GetTxtSrvHeapIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtSrvHeapIndex();
		}
		uint JGraphicTextureUserInterface::GetTxtVectorIndex(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtVectorIndex();
		}
		Graphic::J_GRAPHIC_TEXTURE_TYPE JGraphicTextureUserInterface::GetTxtType(JGraphicTextureInterface& gtI)const noexcept
		{
			return gtI.GetTxtType();
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicTextureUserInterface::GetCpuHandle(JGraphicTextureInterface& gtI) const noexcept
		{
			return gtI.GetCpuHandle();
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicTextureUserInterface::GetGpuHandle(JGraphicTextureInterface& gtI) const noexcept
		{
			return gtI.GetGpuHandle();
		}
	}
}