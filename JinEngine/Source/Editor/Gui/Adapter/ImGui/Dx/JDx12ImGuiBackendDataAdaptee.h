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


#pragma once
#include"../JImGuiBackendDataAdaptee.h"

namespace JinEngine
{
	namespace Editor
	{
		class JDx12ImGuiBackendDataAdaptee : public JImGuiBackendDataAdaptee
		{
		public:
			Graphic::J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final; 
		public:
			std::unique_ptr<Graphic::JGuiInitData> CreateInitData(_In_ Graphic::JGraphicDevice* device,
				_In_ Graphic::JGraphicResourceManager* gm,
				_In_ const Graphic::JGraphicOption& option,
				_In_ const Graphic::GuiIdentification guiIden) final;
			std::unique_ptr<Graphic::JGuiDrawData> CreateDrawData(_In_ Graphic::JGraphicDevice* device,
				_In_ Graphic::JGraphicResourceManager* gm,
				_In_ Graphic::JFrameResource* frame,
				_In_ const Graphic::GuiIdentification guiIden) final;
		};
	}
}