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
			static bool OptionCombo(const std::string& name, 
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
			template<typename Enum>
			static bool EnumCombo(const std::string& name,
				const std::string& uniqueLabel,
				Enum& value)
			{
				int existValue = (int)value;
				bool isChanged = false;
				if (JGui::ComboEnumSet<Enum>(name + uniqueLabel, existValue))
				{
					if ((int)value != existValue)
					{
						value = (Enum)existValue;
						isChanged = true;
					}
				}
				return isChanged;
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
				bool isChanged = JGui::CheckBox("Occlusion Query##JGraphicOptionSetting", option.culling.isOcclusionQueryActivated); 
				isChanged |= JGui::CheckBox("Light Culling##JGraphicOptionSetting", option.culling.isLightCullingActivated);
				isChanged |= JGui::CheckBox("Hzb correct fail##JGraphicOptionSetting", option.culling.allowHZBCorrectFail);
				isChanged |= JGui::CheckBox("Hardware Occlusion Use Occluder##JGraphicOptionSetting", option.culling.allowHDOcclusionUseOccluder);
				isChanged |= JGui::CheckBox("Light Cluster##JGraphicOptionSetting", option.culling.allowLightCluster);
 
				JGui::Text("Cluster");
				isChanged |= OptionCombo("ClusterX", "##JGraphicOptionSetting", option.GetClusterXCount(), litClusterXRange, litClusterXVariation, option.culling.clusterXIndex);
				isChanged |= OptionCombo("ClusterY", "##JGraphicOptionSetting", option.GetClusterYCount(), litClusterYRange, litClusterYVariation, option.culling.clusterYIndex);
				isChanged |= OptionCombo("ClusterZ", "##JGraphicOptionSetting", option.GetClusterZCount(), litClusterZRange, litClusterZVariation, option.culling.clusterZIndex);
				isChanged |= OptionCombo("LightPerCluster", "##JGraphicOptionSetting", option.GetLightPerCluster(), maxLightPerClusterRange, lightPerClusterVariation, option.culling.lightPerClusterIndex);
				isChanged |= JGui::InputFloatClamp("ClusterNear##JGraphicOptionSetting", &option.culling.clusterNear, litClusterNearMin, litClusterNearMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Point Lit Range Offset##JGraphicOptionSetting", &option.culling.clusterPointLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Spot Lit Range Offset##JGraphicOptionSetting", &option.culling.clusterSpotLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("Cluster Rect Lit Range Offset##JGraphicOptionSetting", &option.culling.clusterRectLightRangeOffset, litClusterRangeOffsetMin, litClusterRangeOffsetMax, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				//isChanged |= JGui::CheckBox("HZB Correct Fail##JGraphicOptionSetting", option.allowHZBCorrectFail);
				JGui::Separator();

				JGui::Text("Render");
				isChanged |= JGui::CheckBox("Multithread##JGraphicOptionSetting", option.rendering.allowMultiThread);
				isChanged |= JGui::CheckBox("Draw gui##JGraphicOptionSetting", option.rendering.allowDrawGui);
				isChanged |= JGui::CheckBox("Deferred##JGraphicOptionSetting", option.rendering.allowDeferred);
				isChanged |= JGui::CheckBox("Raytracing##JGraphicOptionSetting", option.rendering.allowRaytracing);
				isChanged |= JGui::CheckBox("MSAA##JGraphicOptionSetting", option.rendering.useMSAA);
				isChanged |= EnumCombo("RenderTarget Format", "##JGraphicOptionSetting", option.rendering.renderTargetFormat);
	 
				JGui::Text("BRDF Microfacet");
				isChanged |= JGui::CheckBox("GGX##JGraphicOptionSetting", option.rendering.useGGXMicrofacet);
				isChanged |= JGui::CheckBox("Beckmann##JGraphicOptionSetting", option.rendering.useBeckmannMicrofacet);
				isChanged |= JGui::CheckBox("BlinnPhong##JGraphicOptionSetting", option.rendering.useBlinnPhongMicrofacet);
				isChanged |= JGui::CheckBox("Isotropy##JGraphicOptionSetting", option.rendering.useIsotropy);
				JGui::Text("BRDF Diffuse");
				isChanged |= JGui::CheckBox("Disney##JGraphicOptionSetting", option.rendering.useDisneyDiffuse);
				isChanged |= JGui::CheckBox("FrostBite##JGraphicOptionSetting", option.rendering.useFrostBiteDiffuse);
				isChanged |= JGui::CheckBox("Hammon##JGraphicOptionSetting", option.rendering.useHammonDiffuse);
				isChanged |= JGui::CheckBox("OrenNayar##JGraphicOptionSetting", option.rendering.useOrenNayarDiffuse);
				isChanged |= JGui::CheckBox("Shirley##JGraphicOptionSetting", option.rendering.useShirleyDiffuse);
				isChanged |= JGui::CheckBox("LambertianDiffuse##JGraphicOptionSetting", option.rendering.useLambertianDiffuse);
				
				JGui::Text("Restir Gi");
				isChanged |= JGui::InputIntClamp("TemporalSampleCount##JGraphicOptionSetting", option.rendering.restir.temporalSampleCount, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
				isChanged |= JGui::InputIntClamp("SpatialSampleCount##JGraphicOptionSetting", option.rendering.restir.spatialSampleCount, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
				isChanged |= JGui::InputIntClamp("SampleMaxAge##JGraphicOptionSetting", option.rendering.restir.sampleMaxAge, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
				isChanged |= JGui::InputFloatClamp("NeighborWeightSumThreshold##JGraphicOptionSetting", option.rendering.restir.neighborWeightSumThreshold, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
				isChanged |= JGui::InputFloatClamp("SpatialWeightAverageThreshold##JGraphicOptionSetting", option.rendering.restir.spatialWeightAverageThreshold, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
				JGui::Separator();

				JGui::Text("Shadow");
				isChanged |= JGui::CheckBox("HighQuality Shadow##JGraphicOptionSetting", option.shadow.useHighQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Middle Quality Shadow##JGraphicOptionSetting", option.shadow.useMiddleQualityShadow);
				JGui::SameLine();
				isChanged |= JGui::CheckBox("Low Quality Shadow##JGraphicOptionSetting", option.shadow.useLowQualityShadow);
				JGui::Separator();
 
				JGui::Text("PostProcess");
				isChanged |= JGui::CheckBox("FXAA##JGraphicOptionSetting", option.postProcess.useFxaa);
				isChanged |= JGui::CheckBox("SSAO##JGraphicOptionSetting", option.postProcess.useSsao);	
				isChanged |= JGui::CheckBox("Use Ssao Inetrleave##JGraphicOptionSetting", option.postProcess.useSsaoInterleave);
				isChanged |= JGui::CheckBox("Use Postprocess##JGraphicOptionSetting", option.postProcess.usePostprocess);
				isChanged |= JGui::CheckBox("Use Hdr##JGraphicOptionSetting", option.postProcess.useHdr);

				uint tmVariation[2]{ 0,1 };
				isChanged |= JGui::CheckBox("Use TomeMapping##JGraphicOptionSetting", option.postProcess.useToneMapping);
				isChanged |= JGui::CheckBox("Use Bloom##JGraphicOptionSetting", option.postProcess.useBloom);
				isChanged |= JGui::CheckBox("Use Histogram Debug##JGraphicOptionSetting", option.postProcess.useHistogramDebug);
				isChanged |= EnumCombo("Exposure", "##JGraphicOptionSetting", option.postProcess.exposureType);
				isChanged |= EnumCombo("BloomQuality", "##JGraphicOptionSetting", option.postProcess.bloomQuality);

				isChanged |= JGui::InputFloatClamp("BloomThreadHold##JGraphicOptionSetting", &option.postProcess.bloomThreadHold, 0.0f, 8.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("BloomStrength##JGraphicOptionSetting", &option.postProcess.bloomStrength, 0.0f, 2.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("BloomUpsampleFactor##JGraphicOptionSetting", &option.postProcess.bloomUpsampleFactor, 0.0f, 1.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);

				isChanged |= JGui::InputFloatClamp("PaperWhiteRatio##JGraphicOptionSetting", &option.postProcess.hdrPaperWhite, 0.1f, 500, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("MaxBrightness##JGraphicOptionSetting", &option.postProcess.maxBrightness, 1.0f, 10000.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);

				isChanged |= JGui::InputFloatClamp("FxaaContrastThreshold##JGraphicOptionSetting", &option.postProcess.fxaaContrastThreshold, 0.05f, 0.5f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("FxaaSubPixelRemoval##JGraphicOptionSetting", &option.postProcess.fxaaSubPixelRemoval, 0.0f, 1.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);

				isChanged |= JGui::InputFloatClamp("MinExposure##JGraphicOptionSetting", &option.postProcess.minExposure, -16.0f, 0.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("MaxExposure##JGraphicOptionSetting", &option.postProcess.maxExposure, 0.0f, 16.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 2.0f);
				isChanged |= JGui::InputFloatClamp("TargetLuminance##JGraphicOptionSetting", &option.postProcess.targetLuminance, 0.001f, 0.99f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 3.0f);
				isChanged |= JGui::InputFloatClamp("AdaptationRate##JGraphicOptionSetting", &option.postProcess.adaptationRate, 0.001f, 1.0f, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE, 3.0f);
				JGui::Separator();

				JGui::Text("Debugging");
				isChanged |= JGui::CheckBox("Debug Out line##JGraphicOptionSetting", option.debugging.allowOutline);
				isChanged |= JGui::CheckBox("Debug light culling##JGraphicOptionSetting", option.debugging.allowDisplayLightCullingResult);
#ifdef DEVELOP
				JGui::Text("Graphic Test");
				//isChanged |= JGui::CheckBox("Test Clear Lit Culling Offset Buffer##JGraphicOptionSettingDiffuse", option.testClearLitCullingOffsetBuffer);
				isChanged |= JGui::CheckBox("Recompile Graphic Shader##JGraphicOptionSetting", option.debugging.requestRecompileGraphicShader);
				isChanged |= JGui::CheckBox("Recompile Light Cluster Shader##JGraphicOptionSetting", option.debugging.requestRecompileLightClusterShader);
				isChanged |= JGui::CheckBox("Recompile Ssao Shader##JGraphicOptionSetting", option.debugging.requestRecompileSsaoShader);
				isChanged |= JGui::CheckBox("Recompile ToneMapping Shader##JGraphicOptionSetting", option.debugging.requestRecompileToneMappingShader);
				isChanged |= JGui::CheckBox("Recompile RtGi Shader Shader##JGraphicOptionSetting", option.debugging.requestRecompileRtGiShader);
				isChanged |= JGui::CheckBox("Recompile RtDenoise Shader Shader##JGraphicOptionSetting", option.debugging.requestRecompileRtDenoiseShader);

				isChanged |= JGui::CheckBox("Test trigger00##JGraphicOptionSetting", option.debugging.testTrigger00);
				isChanged |= JGui::CheckBox("Test trigger01##JGraphicOptionSetting", option.debugging.testTrigger01);
				isChanged |= JGui::CheckBox("Test trigger02##JGraphicOptionSetting", option.debugging.testTrigger02);
				//isChanged |= JGui::CheckBox("Draw ssao by compute shader##JGraphicOptionSetting", option.drawSsaoByComputeShader);
#endif
				JGui::Separator();

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