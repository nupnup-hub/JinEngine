#pragma once
#include"../JImGuiAdaptee.h"

namespace JinEngine
{
	namespace Editor
	{ 
		class JDx12ImGuiAdaptee final: public JImGuiAdaptee
		{  
		protected:
			void IntiailizeBackend(JImGuiInitData* initData) final;
			void ClearBackend() final;
		public:
			void ReBuildGraphicBackend(std::unique_ptr<Graphic::JGuiInitData>&& initData) final;
		public:
			void UpdateGuiBackend() final;
		public:
			void SettingGuiDrawing()final;
			void Draw(std::unique_ptr<Graphic::JGuiDrawData>&& drawData)final;
		public:
			Graphic::J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}