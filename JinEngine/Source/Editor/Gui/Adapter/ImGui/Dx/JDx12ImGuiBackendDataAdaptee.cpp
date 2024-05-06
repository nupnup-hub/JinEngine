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


#include"JDx12ImGuiBackendDataAdaptee.h" 
#include"../../../Data/ImGui/Dx/JDx12ImGuiInitData.h"
#include"../../../Data/ImGui/Dx/JDx12ImGuiDrawData.h"
#include"../../../../../Graphic/Device/Dx/JDx12GraphicDevice.h" 
#include"../../../../../Graphic/GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../../../../Graphic/FrameResource/Dx/JDx12FrameResource.h" 
#include"../../../../../Graphic/JGraphicConstants.h" 
#include"../../../../../Graphic/GraphicResource/Dx/JDx12GraphicResourceConstants.h" 
#include"d3dx12.h"

namespace JinEngine::Editor
{ 
	Graphic::J_GRAPHIC_DEVICE_TYPE JDx12ImGuiBackendDataAdaptee::GetDeviceType()const noexcept
	{
		return Graphic::J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	std::unique_ptr<Graphic::JGuiInitData> JDx12ImGuiBackendDataAdaptee::CreateInitData(_In_ Graphic::JGraphicDevice* device,
		_In_ Graphic::JGraphicResourceManager* gm,
		_In_ const Graphic::JGraphicOption& option,
		_In_ const Graphic::GuiIdentification guiIden)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return nullptr;

		Graphic::JDx12GraphicDevice* dx12Device = static_cast<Graphic::JDx12GraphicDevice*>(device);
		Graphic::JDx12GraphicResourceManager* dx12Gm = static_cast<Graphic::JDx12GraphicResourceManager*>(gm);
 
		return std::make_unique<JDx12ImGuiInitData>(dx12Device->GetSwapChainOutputWindowHandle(),
			dx12Device->GetDevice(),
			dx12Gm->GetDescriptorHeap(Graphic::J_GRAPHIC_BIND_TYPE::SRV),
			Graphic::Constants::GetBackBufferFormat(option.postProcess.useHdr),
			Graphic::Constants::gNumFrameResources);
	}
	std::unique_ptr<Graphic::JGuiDrawData> JDx12ImGuiBackendDataAdaptee::CreateDrawData(_In_ Graphic::JGraphicDevice* device,
		_In_ Graphic::JGraphicResourceManager* gm,
		_In_ Graphic::JFrameResource* frame,
		_In_ const Graphic::GuiIdentification guiIden)
	{
		if (!IsSameDevice(frame))
			return nullptr;

		Graphic::JDx12FrameResource* dx12Frame = static_cast<Graphic::JDx12FrameResource*>(frame);
		return std::make_unique<JDx12ImGuiDrawData>(dx12Frame->GetCmd(Graphic::J_MAIN_THREAD_ORDER::END));
	}
}