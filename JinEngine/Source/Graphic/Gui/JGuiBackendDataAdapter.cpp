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


#include"JGuiBackendDataAdapter.h"
#include"JGuiBackendDataAdaptee.h"
#include"../JGraphicOption.h"
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
		_In_ const JGraphicOption& option,
		_In_ const GuiIdentification guiIden)
	{
		if (device == nullptr || gm == nullptr)
			return nullptr;

		auto mapData = adapteeMap[(uint)option.deviceType].find(guiIden);
		if (mapData == adapteeMap->end())
			return nullptr;
		 
		return mapData->second->CreateInitData(device, gm, option, guiIden);
	}
	std::unique_ptr<JGuiDrawData> JGuiBackendDataAdapter::CreateDrawData(_In_ JGraphicDevice* device,
		_In_ JGraphicResourceManager* gm,
		_In_ JFrameResource* frame,
		_In_ const JGraphicOption& option,
		_In_ const GuiIdentification guiIden)
	{
		if (device == nullptr || gm == nullptr || frame == nullptr)
			return nullptr;

		auto mapData = adapteeMap[(uint)option.deviceType].find(guiIden);
		if (mapData == adapteeMap->end())
			return nullptr;

		return mapData->second->CreateDrawData(device, gm, frame, guiIden);
	}
}