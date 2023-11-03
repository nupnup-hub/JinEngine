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
				isChanged |= JGui::CheckBox("Use DirectionalLight Pcm##JGraphicOptionSetting", option.useDirectionalLightPcm);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use DirectionalLight Pcm HighQuality##JGraphicOptionSetting", option.useDirectionalLightPcmHighQuality);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use DirectionalLight Pcss##JGraphicOptionSetting", option.useDirectionalLightPcss);
				 
				isChanged |= JGui::CheckBox("Use PointLight Pcm##JGraphicOptionSetting", option.usePointLightPcm);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use PointLight Pcm HighQuality##JGraphicOptionSetting", option.usePointLightPcmHighQuality);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use PointLight Pcss##JGraphicOptionSetting", option.usePointLightPcss);
				 
				isChanged |= JGui::CheckBox("Use SpotLight Pcm##JGraphicOptionSetting", option.useSpotLightPcm);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use SpotLight Pcm HighQuality##JGraphicOptionSetting", option.useSpotLightPcmHighQuality);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Use SpotLight Pcss##JGraphicOptionSetting", option.useSpotLightPcss);
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