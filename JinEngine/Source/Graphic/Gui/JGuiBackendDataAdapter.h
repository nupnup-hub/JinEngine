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
#include"JGuiData.h"
#include"JGuiConstants.h"
#include"../Device/JGraphicDeviceType.h" 

namespace JinEngine 
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JFrameResource;
		class JGuiBackendDataAdaptee;
		struct JGraphicOption;

		//use communication
		//create data for guibackend(update, draw)
		class JGuiBackendDataAdapter
		{ 
		private:  
			using GuiAdapteeMap = std::unordered_map<GuiIdentification, std::unique_ptr<JGuiBackendDataAdaptee>>;
		private:
			GuiAdapteeMap adapteeMap[(uint)J_GRAPHIC_DEVICE_TYPE::COUNT];
		public:
			void AddAdaptee(std::unique_ptr<JGuiBackendDataAdaptee>&& newAdaptee);
		public:
			std::unique_ptr<JGuiInitData> CreateInitData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm, 
				_In_ const JGraphicOption& option,
				_In_ const GuiIdentification guiIden);
			std::unique_ptr<JGuiDrawData> CreateDrawData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ JFrameResource* frame,
				_In_ const JGraphicOption& option,
				_In_ const GuiIdentification guiIden);
		};
	}
}