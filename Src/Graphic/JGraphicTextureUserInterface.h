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
			uint CallGetTxtWidth(JGraphicTextureInterface& gtI)const noexcept;
			uint CallGetTxtHeight(JGraphicTextureInterface& gtI)const noexcept;
			//n(predefined resource) ~ m
			uint CallGetTxtRtvHeapIndex(JGraphicTextureInterface& gtI)const noexcept;
			//n(predefined resource) ~ m
			uint CallGetTxtDsvHeapIndex(JGraphicTextureInterface& gtI)const noexcept;
			//n(predefined resource) ~ m
			uint CallGetTxtSrvHeapIndex(JGraphicTextureInterface& gtI)const noexcept;
			//0 ~ n
			uint CallGetTxtVectorIndex(JGraphicTextureInterface& gtI)const noexcept;
			Graphic::J_GRAPHIC_TEXTURE_TYPE CallGetTxtType(JGraphicTextureInterface& gtI)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE CallGetCpuHandle(JGraphicTextureInterface& gtI) const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE CallGetGpuHandle(JGraphicTextureInterface& gtI) const noexcept;
		};
	}
}