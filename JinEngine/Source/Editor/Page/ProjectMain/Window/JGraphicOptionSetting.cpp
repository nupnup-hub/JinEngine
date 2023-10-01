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
				JGui::Text("Occlusion");
				bool isChanged = JGui::CheckBox("Occlusion Query##JGraphicOptionSetting", option.isOcclusionQueryActivated);
				isChanged |= JGui::CheckBox("Hardware Occlusion##JGraphicOptionSetting", option.isHDOcclusionAcitvated);
				isChanged |= JGui::CheckBox("HZB Occlusion##JGraphicOptionSetting", option.isHZBOcclusionActivated);
				//isChanged |= JGui::CheckBox("HZB Correct Fail##JGraphicOptionSetting", option.allowHZBCorrectFail);
				JGui::Separator();
				JGui::Text("Render");
				isChanged |= JGui::CheckBox("Allow Debug Out line##JGraphicOptionSetting", option.allowDebugOutline);
				isChanged |= JGui::CheckBox("Allow Multithread##JGraphicOptionSetting", option.allowMultiThread); 
				JGui::Separator();
				JGui::Text("Shadow");
				isChanged |= JGui::CheckBox("Use DirectionalLight Pcss##JGraphicOptionSetting", option.useDirectionalLightPcss);
				isChanged |= JGui::CheckBox("Use DirectionalLight Pcm##JGraphicOptionSetting", option.useDirectionalLightPcm);
				isChanged |= JGui::CheckBox("Use PointLight Pcm##JGraphicOptionSetting", option.usePointLightPcm);
				isChanged |= JGui::CheckBox("Use SpotLight Pcm##JGraphicOptionSetting", option.useSpotLightPcm);
				JGui::Separator();
				JGui::Text("Test");
				isChanged |= JGui::CheckBox("Allow Hd Dist Test##JGraphicOptionSetting", option.allowHdDistTest);
				if (isChanged)
					JGraphic::Instance().SetGraphicOption(option);
			}
			CloseWindow();
		}
	}
}