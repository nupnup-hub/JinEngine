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
				_In_ const Graphic::GuiIdentification guiIden) final;
			std::unique_ptr<Graphic::JGuiDrawData> CreateDrawData(_In_ Graphic::JGraphicDevice* device,
				_In_ Graphic::JGraphicResourceManager* gm,
				_In_ Graphic::JFrameResource* frame,
				_In_ const Graphic::GuiIdentification guiIden) final;
		};
	}
}