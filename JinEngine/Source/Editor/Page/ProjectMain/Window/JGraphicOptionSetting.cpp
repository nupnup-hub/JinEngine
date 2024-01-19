#include"JGraphicOptionSetting.h"
#include"../../JEditorAttribute.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../Gui/JGui.h"
#include"../../../../Graphic/JGraphic.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			static bool ClusterCombo(const std::string& name, 
				const std::string& uniqueLabel,
				const uint preValue,
				const uint* value,
				uint valueCount,
				uint& selectedIndex)
			{
				bool isSelected = false;				
				if (JGui::BeginCombo(name + uniqueLabel, std::to_string(preValue)))
				{ 
					for (uint i = 0; i < valueCount; ++i)
					{ 
						if (JGui::Selectable(std::to_string(value[i]) + uniqueLabel))
						{
							if (selectedIndex != i)
							{
								selectedIndex = i;
								isSelected = true;
							}
						}
					}
					JGui::EndCombo();
				}
				return isSelected;
			}
		}

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
			using namespace Graphic::Constants;
			EnterWindow(J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			if (IsActivated())
			{
				UpdateMouseClick();
				Graphic::JGraphicOption option = JGraphic::Instance().GetGraphicOption();
				JGui::Text("Culling");
				bool isChanged = JGui::CheckBox("Occlusion Query##JGraphicOptionSetting", option.isOcclusionQueryActivated); 
				isChanged |= JGui::CheckBox("Light Culling##JGraphicOptionSetting", option.isLightCullingActivated);
				isChanged |= JGui::CheckBox("Hzb correct fail##JGraphicOptionSetting", option.allowHZBCorrectFail);
				isChanged |= JGui::CheckBox("Hardware Occlusion Use Occluder##JGraphicOptionSetting", option.allowHDOcclusionUseOccluder);
				//isChanged |= JGui::CheckBox("HZB Correct Fail##JGraphicOptionSetting", option.allowHZBCorrectFail);
				JGui::Separator();

				JGui::Text("Debugging");
				isChanged |= JGui::CheckBox("Debug Out line##JGraphicOptionSetting", option.allowDebugOutline);
				isChanged |= JGui::CheckBox("Debug light culling##JGraphicOptionSetting", option.allowDebugLightCulling);
				JGui::Separator();

				JGui::Text("Render");
				isChanged |= JGui::CheckBox("Multithread##JGraphicOptionSetting", option.allowMultiThread); 	 
				isChanged |= JGui::CheckBox("Draw gui##JGraphicOptionSetting", option.allowDrawGui);
				isChanged |= JGui::CheckBox("Light Cluster##JGraphicOptionSetting", option.allowLightCluster);
				isChanged |= JGui::CheckBox("Deferred##JGraphicOptionSetting", option.allowDeferred);
				isChanged |= JGui::CheckBox("MSAA##JGraphicOptionSetting", option.useMSAA);
				JGui::Separator();

				JGui::Text("Shadow");
				isChanged |= JGui::CheckBox("HighQuality Shadow##JGraphicOptionSetting", option.useHighQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Middle Quality Shadow##JGraphicOptionSetting", option.useMiddleQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Low Quality Shadow##JGraphicOptionSetting", option.useLowQualityShadow);
				JGui::Separator();

				JGui::Text("Ssao");
				isChanged |= JGui::CheckBox("SSAO##JGraphicOptionSetting", option.useSsao);			
				JGui::Separator();

				JGui::Text("Cluster");
				isChanged |= ClusterCombo("ClusterX", "##JGraphicOptionSetting", option.GetClusterXCount(), litClusterXRange, litClusterXVariation, option.clusterXIndex);
				isChanged |= ClusterCombo("ClusterY", "##JGraphicOptionSetting", option.GetClusterYCount(), litClusterYRange, litClusterYVariation, option.clusterYIndex);
				isChanged |= ClusterCombo("ClusterZ", "##JGraphicOptionSetting", option.GetClusterZCount(), litClusterZRange, litClusterZVariation, option.clusterZIndex);
				isChanged |= ClusterCombo("LightPerCluster", "##JGraphicOptionSetting", option.GetLightPerCluster(), maxLightPerClusterRange, lightPerClusterVariation, option.lightPerClusterIndex);
				isChanged |= JGui::InputFloatClamp("ClusterNear##JGraphicOptionSetting", &option.clusterNear, litClusterNearMin, litClusterNearMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Point Lit Range Offset##JGraphicOptionSetting", &option.clusterPointLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Spot Lit Range Offset##JGraphicOptionSetting", &option.clusterSpotLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Rect Lit Range Offset##JGraphicOptionSetting", &option.clusterRectLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				JGui::Separator();

				JGui::Text("BRDF Masking");
				isChanged |= JGui::CheckBox("Smith##JGraphicOptionSetting", option.useSmithMasking);
				isChanged |= JGui::CheckBox("Torrance##JGraphicOptionSetting", option.useTorranceMaskig);
				JGui::Text("BRDF NDF");
				isChanged |= JGui::CheckBox("GGX##JGraphicOptionSetting", option.useGGXNDF);
				isChanged |= JGui::CheckBox("Beckmann##JGraphicOptionSetting", option.useBeckmannNDF);
				isChanged |= JGui::CheckBox("BlinnPhong##JGraphicOptionSetting", option.useBlinnPhongNDF);
				isChanged |= JGui::CheckBox("Isotropy##JGraphicOptionSetting", option.useIsotropy);
				JGui::Text("BRDF Diffuse");
				isChanged |= JGui::CheckBox("Disney##JGraphicOptionSetting", option.useDisneyDiffuse);
				isChanged |= JGui::CheckBox("Hammon##JGraphicOptionSetting", option.useHammonDiffuse);
				isChanged |= JGui::CheckBox("OrenNayar##JGraphicOptionSetting", option.useOrenNayarDiffuse);
				isChanged |= JGui::CheckBox("Shirley##JGraphicOptionSetting", option.useShirleyDiffuse);
				isChanged |= JGui::CheckBox("LambertianDiffuse##JGraphicOptionSetting", option.useLambertianDiffuse);
				JGui::Separator();
#ifdef DEVELOP
				JGui::Text("Graphic Test");
				//isChanged |= JGui::CheckBox("Test Clear Lit Culling Offset Buffer##JGraphicOptionSettingDiffuse", option.testClearLitCullingOffsetBuffer);
				isChanged |= JGui::CheckBox("Recompile Graphic Shader##JGraphicOptionSetting", option.requestRecompileGraphicShader);
				isChanged |= JGui::CheckBox("Recompile Light Cluster Shader##JGraphicOptionSetting", option.requestRecompileLightClusterShader);
				isChanged |= JGui::CheckBox("Recompile Ssao Shader##JGraphicOptionSetting", option.requestRecompileSsaoShader);
				isChanged |= JGui::CheckBox("Use Ssao Inetrleave##JGraphicOptionSetting", option.useSsaoInterleave);

				//isChanged |= JGui::CheckBox("Draw ssao by compute shader##JGraphicOptionSetting", option.drawSsaoByComputeShader);
#endif
				if (isChanged)
				{ 
					using SetOptFunc = Core::JSFunctorType<void, Graphic::JGraphicOption>::Functor;
					auto setOptionLam = [](Graphic::JGraphicOption option)
					{
						JGraphic::Instance().SetGraphicOption(option);
					};
					auto doBind = Core::UniqueBind(std::make_unique<SetOptFunc>(setOptionLam), std::forward<Graphic::JGraphicOption>(option));

					auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(doBind), GetOwnerPageType()));
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, evStruct);
				}
			}
			CloseWindow();
		}
	}
}