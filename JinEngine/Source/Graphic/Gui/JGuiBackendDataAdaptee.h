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
#include"JGuiConstants.h"
#include"../Device/JGraphicDeviceUser.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JFrameResource;
		struct JGraphicOption;
		struct JGuiInitData; 
		struct JGuiDrawData;
		class JGuiBackendDataAdaptee : public JGraphicDeviceUser
		{ 
		public:
			virtual J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept = 0;
			virtual GuiIdentification GetGuiIdentification()const noexcept = 0;
		public:
			virtual std::unique_ptr<JGuiInitData> CreateInitData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ const JGraphicOption& option,
				_In_ const GuiIdentification guiIden) = 0;
			virtual std::unique_ptr<JGuiDrawData> CreateDrawData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ JFrameResource* frame,
				_In_ const GuiIdentification guiIden) = 0;
		};
	}
}