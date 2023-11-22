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
				isChanged |= JGui::CheckBox("Use HighQuality Shadow##JGraphicOptionSetting", option.useHighQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use Middle Quality Shadow##JGraphicOptionSetting", option.useMiddleQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use Low Quality Shadow##JGraphicOptionSetting", option.useLowQualityShadow);
				JGui::Separator();

				JGui::Text("Ssao");
				isChanged |= JGui::CheckBox("Use SSAO##JGraphicOptionSetting", option.useSsao);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use HBAO##JGraphicOptionSetting", option.useHbao);
				JGui::Separator();

				JGui::Text("BRDF Masking");
				isChanged |= JGui::CheckBox("Use Smith##JGraphicOptionSetting", option.useSmithMasking);
				isChanged |= JGui::CheckBox("Use Torrance##JGraphicOptionSetting", option.useTorranceMaskig);
				JGui::Text("BRDF NDF");
				isChanged |= JGui::CheckBox("Use GGX##JGraphicOptionSetting", option.useGGXNDF);
				isChanged |= JGui::CheckBox("Use Beckmann##JGraphicOptionSetting", option.useBeckmannNDF);
				isChanged |= JGui::CheckBox("Use BlinnPhong##JGraphicOptionSetting", option.useBlinnPhongNDF);
				isChanged |= JGui::CheckBox("Use Isotropy##JGraphicOptionSetting", option.useIsotropy);
				JGui::Text("BRDF Diffuse");
				isChanged |= JGui::CheckBox("Use Disney##JGraphicOptionSetting", option.useDisneyDiffuse);
				isChanged |= JGui::CheckBox("Use Hammon##JGraphicOptionSetting", option.useHammonDiffuse);
				isChanged |= JGui::CheckBox("Use Shirley##JGraphicOptionSetting", option.useShirleyDiffuse);
				isChanged |= JGui::CheckBox("Use LambertianDiffuse##JGraphicOptionSettingDiffuse", option.useLambertianDiffuse);
				if (isChanged)
					JGraphic::Instance().SetGraphicOption(option);
			}
			CloseWindow();
		}
	}
}