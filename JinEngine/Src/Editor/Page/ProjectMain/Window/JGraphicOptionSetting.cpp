#include"JGraphicOptionSetting.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Graphic/JGraphic.h"

namespace JinEngine
{
	namespace Editor
	{
		void JGraphicOptionSetting::GraphicOptionOnScreen()
		{ 
			JImGuiImpl::BeginWindow("GraphicOption##GraphicOptionSetting", 0, ImGuiWindowFlags_NoDocking);
			Graphic::JGraphicOption option = JGraphic::Instance().GetGraphicOption();
			bool isChanged = JImGuiImpl::CheckBox("Occlusion Query##JGraphicOptionSetting", option.isOcclusionQueryActivated); 
			isChanged |= JImGuiImpl::CheckBox("Hardware Occlusion##JGraphicOptionSetting", option.isHDOcclusionAcitvated);
			isChanged |= JImGuiImpl::CheckBox("HZB Occlusion##JGraphicOptionSetting", option.isHZBOcclusionActivated);
			isChanged |= JImGuiImpl::SliderFloat("Update Frequency##JGraphicOptionSetting", &option.occUpdateFrequency, 0.01f, 2.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			JImGuiImpl::EndWindow();
			if (isChanged)
				JGraphic::Instance().SetGraphicOption(option);
		}
	}
}