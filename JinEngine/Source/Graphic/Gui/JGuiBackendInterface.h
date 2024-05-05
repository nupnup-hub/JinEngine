#pragma once
#include"JGuiData.h"
#include"JGuiConstants.h"  

namespace JinEngine
{
	namespace Graphic
	{ 
		class JGuiBackendInterface
		{  
		public:
			virtual GuiIdentification GetGuiIdentification()const noexcept = 0;   
		public:
			virtual void ReBuildGraphicBackend(std::unique_ptr<Graphic::JGuiInitData>&& initData) = 0;
		public:
			virtual void SettingGuiDrawing() = 0;
			virtual void Draw(std::unique_ptr<JGuiDrawData>&& drawData) = 0;
		};
	}
}