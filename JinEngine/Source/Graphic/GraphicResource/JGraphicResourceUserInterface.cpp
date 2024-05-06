/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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