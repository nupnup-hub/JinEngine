#pragma once 
#include"JGuiConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JFrameResource;
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
				_In_ const GuiIdentification guiIden) = 0;
			virtual std::unique_ptr<JGuiDrawData> CreateDrawData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ JFrameResource* frame,
				_In_ const GuiIdentification guiIden) = 0;
		};
	}
}