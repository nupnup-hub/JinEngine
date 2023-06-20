#include"JGraphicOptionSetting.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Graphic/JGraphic.h"

namespace JinEngine
{
	namespace Editor
	{
		void JGraphicOptionSetting::Update()
		{ 
			JImGuiImpl::BeginWindow("GraphicOption##GraphicOptionSetting", GetOpenPtr(), ImGuiWindowFlags_NoDocking);
			Graphic::JGraphicOption option = JGraphic::Instance().GetGraphicOption();
			bool isChanged = JImGuiImpl::CheckBox("Occlusion Query##JGraphicOptionSetting", option.isOcclusionQueryActivated); 
			isChanged |= JImGuiImpl::CheckBox("Hardware Occlusion##JGraphicOptionSetting", option.isHDOcclusionAcitvated);
			isChanged |= JImGuiImpl::CheckBox("HZB Occlusion##JGraphicOptionSetting", option.isHZBOcclusionActivated); 
			isChanged |= JImGuiImpl::CheckBox("Allow Debug Out line##JGraphicOptionSetting", option.allowDebugOutline); 
			isChanged |= JImGuiImpl::CheckBox("Allow Multithread##JGraphicOptionSetting", option.allowMultiThread);
			JImGuiImpl::EndWindow();
			if (isChanged)
				JGraphic::Instance().SetGraphicOption(option);
		}
	}
}