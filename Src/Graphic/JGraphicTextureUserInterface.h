#pragma once
#include"../Core/JDataType.h"
#include"JGraphicTextureType.h"
#include"JGraphicBufInterface.h"

struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicTextureInterface;
		class JGraphicTextureUserInterface : public JGraphicBufUserInterface
		{
		protected:
			uint GetTxtWidth(JGraphicTextureInterface& gtI)const noexcept;
			uint GetTxtHeight(JGraphicTextureInterface& gtI)const noexcept;
			uint GetTxtRtvHeapIndex(JGraphicTextureInterface& gtI)const noexcept;
			uint GetTxtDsvHeapIndex(JGraphicTextureInterface& gtI)const noexcept;
			uint GetTxtSrvHeapIndex(JGraphicTextureInterface& gtI)const noexcept; 
			uint GetTxtVectorIndex(JGraphicTextureInterface& gtI)const noexcept;
			Graphic::J_GRAPHIC_TEXTURE_TYPE GetTxtType(JGraphicTextureInterface& gtI)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(JGraphicTextureInterface& gtI) const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(JGraphicTextureInterface& gtI) const noexcept;
		};
	}
}