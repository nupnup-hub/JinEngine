#include"JDx12ImGuiAdaptee.h" 
#include"../../../Data/ImGui/Dx/JDx12ImGuiInitData.h"
#include"../../../Data/ImGui/Dx/JDx12ImGuiDrawData.h"
#include"../../../../../ThirdParty/imgui/imgui_impl_dx12.h"
#include"../../../../../ThirdParty/imgui/imgui_impl_win32.h"
 
namespace JinEngine::Editor
{
	void JDx12ImGuiAdaptee::IntiailizeBackend(JImGuiInitData* initData)
	{
		if (initData == nullptr)
			return;
		 
		if (!IsSameGui(initData->GetGuiIdentification()) || !IsSameDevice(initData))
			return;

		JDx12ImGuiInitData* dx12InitData = static_cast<JDx12ImGuiInitData*>(initData);		 
		ImGui_ImplWin32_Init(dx12InitData->windowHandle); 
		ImGui_ImplDX12_Init(dx12InitData->device, dx12InitData->numOfFrameCount,
			dx12InitData->backbufferFormat,
			dx12InitData->srvHeap,
			dx12InitData->srvHeap->GetCPUDescriptorHandleForHeapStart(),
			dx12InitData->srvHeap->GetGPUDescriptorHandleForHeapStart());
	}
	void JDx12ImGuiAdaptee::ClearBackend()
	{ 
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
	} 
	void JDx12ImGuiAdaptee::ReBuildGraphicBackend(std::unique_ptr<Graphic::JGuiInitData>&& initData) 
	{
		if (initData == nullptr)
			return;

		JDx12ImGuiInitData* dx12InitData = static_cast<JDx12ImGuiInitData*>(initData.get());
		if (!IsSameGui(initData->GetGuiIdentification()) || !IsSameDevice(dx12InitData))
			return;
		 
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplDX12_Init(dx12InitData->device, dx12InitData->numOfFrameCount,
			dx12InitData->backbufferFormat,
			dx12InitData->srvHeap,
			dx12InitData->srvHeap->GetCPUDescriptorHandleForHeapStart(),
			dx12InitData->srvHeap->GetGPUDescriptorHandleForHeapStart());
	}
	void JDx12ImGuiAdaptee::UpdateGuiBackend() 
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	} 
	void JDx12ImGuiAdaptee::SettingGuiDrawing()
	{ 
		ImGui::Render(); 
	}
	void JDx12ImGuiAdaptee::Draw(std::unique_ptr<Graphic::JGuiDrawData>&& drawData)
	{
		if (!IsSameDevice(drawData.get()))
			return;
		 
		JDx12ImGuiDrawData* dx12DrawData = static_cast<JDx12ImGuiDrawData*>(drawData.get());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12DrawData->cmd); 
	}
	Graphic::J_GRAPHIC_DEVICE_TYPE JDx12ImGuiAdaptee::GetDeviceType()const noexcept
	{
		return Graphic::J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}