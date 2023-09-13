#include"JGraphicOptionSetting.h"
#include"../../JEditorAttribute.h"
#include"../../../Gui/JGui.h"
#include"../../../../Graphic/JGraphic.h"

namespace JinEngine
{
	namespace Editor
	{
		JGraphicOptionSetting::JGraphicOptionSetting(const std::string name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{}

		J_EDITOR_WINDOW_TYPE JGraphicOptionSetting::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::GRAPHIC_OPTION;
		}
		void JGraphicOptionSetting::UpdateWindow()
		{  
			EnterWindow(J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			if (IsActivated())
			{
				UpdateMouseClick();
				Graphic::JGraphicOption option = JGraphic::Instance().GetGraphicOption();
				bool isChanged = JGui::CheckBox("Occlusion Query##JGraphicOptionSetting", option.isOcclusionQueryActivated);
				isChanged |= JGui::CheckBox("Hardware Occlusion##JGraphicOptionSetting", option.isHDOcclusionAcitvated);
				isChanged |= JGui::CheckBox("HZB Occlusion##JGraphicOptionSetting", option.isHZBOcclusionActivated);
				//isChanged |= JGui::CheckBox("HZB Correct Fail##JGraphicOptionSetting", option.allowHZBCorrectFail);
				isChanged |= JGui::CheckBox("Allow Debug Out line##JGraphicOptionSetting", option.allowDebugOutline);
				isChanged |= JGui::CheckBox("Allow Multithread##JGraphicOptionSetting", option.allowMultiThread); 
				if (isChanged)
					JGraphic::Instance().SetGraphicOption(option);
			}
			CloseWindow();
		}
	}
}