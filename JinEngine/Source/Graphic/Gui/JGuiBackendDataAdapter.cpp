#include"JGuiBackendDataAdapter.h"
#include"JGuiBackendDataAdaptee.h"
#include<Windows.h>

namespace JinEngine::Graphic
{
	void JGuiBackendDataAdapter::AddAdaptee(std::unique_ptr<JGuiBackendDataAdaptee>&& newAdaptee)
	{
		if (newAdaptee != nullptr)
		{
			const J_GRAPHIC_DEVICE_TYPE deviceType = newAdaptee->GetDeviceType();
			const GuiIdentification guiType = newAdaptee->GetGuiIdentification();
			 
			adapteeMap[(uint)deviceType].emplace(guiType, std::move(newAdaptee));
		}
	}
	std::unique_ptr<JGuiInitData> JGuiBackendDataAdapter::CreateInitData(_In_ JGraphicDevice* device,
		_In_ JGraphicResourceManager* gm,
		_In_ const J_GRAPHIC_DEVICE_TYPE deviceType,
		_In_ const GuiIdentification guiIden)
	{
		if (device == nullptr || gm == nullptr)
			return nullptr;

		auto mapData = adapteeMap[(uint)deviceType].find(guiIden);
		if (mapData == adapteeMap->end())
			return nullptr;
		 
		return mapData->second->CreateInitData(device, gm, guiIden);
	}
	std::unique_ptr<JGuiDrawData> JGuiBackendDataAdapter::CreateDrawData(_In_ JGraphicDevice* device,
		_In_ JGraphicResourceManager* gm,
		_In_ JFrameResource* frame,
		_In_ const J_GRAPHIC_DEVICE_TYPE deviceType,
		_In_ const GuiIdentification guiIden)
	{
		if (device == nullptr || gm == nullptr || frame == nullptr)
			return nullptr;

		auto mapData = adapteeMap[(uint)deviceType].find(guiIden);
		if (mapData == adapteeMap->end())
			return nullptr;

		return mapData->second->CreateDrawData(device, gm, frame, guiIden);
	}
}