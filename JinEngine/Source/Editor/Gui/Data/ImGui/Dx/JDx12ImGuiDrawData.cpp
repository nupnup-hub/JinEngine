#include"JDx12ImGuiDrawData.h"

namespace JinEngine::Editor
{
	JDx12ImGuiDrawData::JDx12ImGuiDrawData(ID3D12GraphicsCommandList* cmd)
		:cmd(cmd)
	{}
	Graphic::J_GRAPHIC_DEVICE_TYPE JDx12ImGuiDrawData::GetDeviceType()const noexcept
	{
		return Graphic::J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}