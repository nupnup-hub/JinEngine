/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JGraphicOption.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Application/Project/JApplicationProject.h"


namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicOption::Culling::LightCullingDependencyChanged(const JGraphicOption& rhs)const noexcept
		{
			return isLightCullingActivated != rhs.culling.isLightCullingActivated ||
				allowLightCluster != rhs.culling.allowLightCluster ||
				clusterXIndex != rhs.culling.clusterXIndex ||
				clusterYIndex != rhs.culling.clusterYIndex ||
				clusterZIndex != rhs.culling.clusterZIndex ||
				lightPerClusterIndex != rhs.culling.lightPerClusterIndex ||
				clusterNear != rhs.culling.clusterNear ||
				clusterPointLightRangeOffset != rhs.culling.clusterPointLightRangeOffset ||
				clusterSpotLightRangeOffset != rhs.culling.clusterSpotLightRangeOffset ||
				clusterRectLightRangeOffset != rhs.culling.clusterRectLightRangeOffset;
		}

		bool JGraphicOption::Rendering::Restir::operator==(const Restir& rhs)const noexcept
		{
			return bounceCount == rhs.bounceCount &&
				temporalSampleCount == rhs.temporalSampleCount &&
				spatialSampleCount == rhs.spatialSampleCount &&
				sampleMaxAge == rhs.sampleMaxAge &&
				neighborWeightSumThreshold == rhs.neighborWeightSumThreshold &&
				spatialWeightAverageThreshold == rhs.spatialWeightAverageThreshold && 
				useDenoiser == rhs.useDenoiser;
		}
		bool JGraphicOption::Rendering::Restir::operator!=(const Restir& rhs)const noexcept
		{
			return bounceCount != rhs.bounceCount ||
				temporalSampleCount != rhs.temporalSampleCount ||
				spatialSampleCount != rhs.spatialSampleCount ||
				sampleMaxAge != rhs.sampleMaxAge ||
				neighborWeightSumThreshold != rhs.neighborWeightSumThreshold ||
				spatialWeightAverageThreshold != rhs.spatialWeightAverageThreshold || 
				useDenoiser != rhs.useDenoiser;
		}

		uint JGraphicOption::GetClusterTotalCount()const noexcept
		{
			return GetClusterXCount() * GetClusterYCount() * GetClusterZCount();
		}
		uint JGraphicOption::GetClusterIndexCount()const noexcept
		{
			return GetClusterTotalCount() * GetLightPerCluster();
		}
		uint JGraphicOption::GetClusterXCount()const noexcept
		{
			return Constants::litClusterXRange[culling.clusterXIndex];
		}
		uint JGraphicOption::GetClusterYCount()const noexcept
		{
			return Constants::litClusterYRange[culling.clusterYIndex];
		}
		uint JGraphicOption::GetClusterZCount()const noexcept
		{
			return Constants::litClusterZRange[culling.clusterZIndex];
		}
		uint JGraphicOption::GetLightPerCluster()const noexcept
		{
			return Constants::maxLightPerClusterRange[culling.lightPerClusterIndex];
		}
		bool JGraphicOption::IsOcclusionActivated()const noexcept
		{
			return culling.isOcclusionQueryActivated;
		}  
		bool JGraphicOption::IsPostProcessActivated()const noexcept
		{
			return postProcess.usePostprocess && (postProcess.useToneMapping || postProcess.useFxaa);
		}
		bool JGraphicOption::CanUseRtGi()const noexcept
		{
			return rendering.allowDeferred && rendering.allowRaytracing;
		}
		bool JGraphicOption::CanUseSSAO()const noexcept
		{ 
			return rendering.allowDeferred && postProcess.useSsao;
		}  
		void JGraphicOption::Load()
		{
			JFileIOTool tool;
			const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
			if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return;
 
			tool.PushExistStack("--Culling--");
			JFileIOHelper::LoadAtomicData(tool, culling.isOcclusionQueryActivated, "IsOcclusionQueryActivated:");
			JFileIOHelper::LoadAtomicData(tool, culling.isLightCullingActivated, "IsLightCullingActivated:");
			JFileIOHelper::LoadAtomicData(tool, culling.allowHZBCorrectFail, "AllowHZBCorrectFail:");
			JFileIOHelper::LoadAtomicData(tool, culling.allowHDOcclusionUseOccluder, "AllowHDOcclusionUseOccluder:");
			JFileIOHelper::LoadAtomicData(tool, culling.allowLightCluster, "AllowLightCluster:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterXIndex, "ClusterXIndex:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterYIndex, "ClusterYIndex:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterZIndex, "ClusterZIndex:");
			JFileIOHelper::LoadAtomicData(tool, culling.lightPerClusterIndex, "LightPerClusterIndex:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterNear, "ClusterNear:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterPointLightRangeOffset, "ClusterPointLightRangeOffset:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterSpotLightRangeOffset, "ClusterSpotLightRangeOffset:");
			JFileIOHelper::LoadAtomicData(tool, culling.clusterRectLightRangeOffset, "ClusterRectLightRangeOffset:");
			tool.PopStack();
 
			tool.PushExistStack("--Rendering--");
			JFileIOHelper::LoadAtomicData(tool, rendering.allowMultiThread, "AllowMultiThread:");
			JFileIOHelper::LoadAtomicData(tool, rendering.allowDrawGui, "AllowDrawGui:");
			JFileIOHelper::LoadAtomicData(tool, rendering.allowDeferred, "AllowDeferred:");
			JFileIOHelper::LoadAtomicData(tool, rendering.allowRaytracing, "AllowRaytracing:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useMSAA, "Msaa:");
			JFileIOHelper::LoadEnumData(tool, rendering.renderTargetFormat, "RenderTargetFormat:");

			JFileIOHelper::LoadAtomicData(tool, rendering.useGGXMicrofacet, "UseGGXMicrofacet:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useBeckmannMicrofacet, "UseBeckmannMicrofacet:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useBlinnPhongMicrofacet, "UseBlinnPhongMicrofacet:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useIsotropy, "UseIsotropy:");

			JFileIOHelper::LoadAtomicData(tool, rendering.useDisneyDiffuse, "UseDisneyDiffuse:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useFrostBiteDiffuse, "UseFrostBiteDiffuse:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useHammonDiffuse, "UseHammonDiffuse:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useOrenNayarDiffuse, "UseOrenNayarDiffuse:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useShirleyDiffuse, "UseShirleyDiffuse:");
			JFileIOHelper::LoadAtomicData(tool, rendering.useLambertianDiffuse, "UseLambertianDiffuse:");

			JFileIOHelper::LoadAtomicData(tool, rendering.restir.bounceCount.GetRef(), "RestirBounceCount:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.temporalSampleCount.GetRef(), "RestirTemporalSampleCount:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.spatialSampleCount.GetRef(), "RestirSpatialSampleCount:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.sampleMaxAge.GetRef(), "RestirSampleMaxAge:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.neighborWeightSumThreshold.GetRef(), "RestirNeighborWeightSumThreshold:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.spatialWeightAverageThreshold.GetRef(), "RestirSpatialWeightAverageThreshold:");
			JFileIOHelper::LoadAtomicData(tool, rendering.restir.useDenoiser, "ResitrUseDenoiser:");
			tool.PopStack();

			tool.PushExistStack("--Shadow--");
			JFileIOHelper::LoadAtomicData(tool, shadow.useHighQualityShadow, "UseHighQualityShadow:");
			JFileIOHelper::LoadAtomicData(tool, shadow.useMiddleQualityShadow, "UseMiddleQualityShadow:");
			JFileIOHelper::LoadAtomicData(tool, shadow.useLowQualityShadow, "UseLowQualityShadow:");
			tool.PopStack();

			tool.PushExistStack("--PostProcess--");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useFxaa, "UseFxaa:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useSsao, "UseSsao:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useSsaoInterleave, "UseSsaoInterleave:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.usePostprocess, "UsePostprocess:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useHdr, "UseHdr:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useToneMapping, "UseToneMapping:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useBloom, "UseBloom:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.useHistogramDebug, "UseHistogramDebug:");

			JFileIOHelper::LoadEnumData(tool, postProcess.exposureType, "ExposureType:");
			JFileIOHelper::LoadEnumData(tool, postProcess.bloomQuality, "BloomQuality:");

			JFileIOHelper::LoadAtomicData(tool, postProcess.bloomThreadHold, "BloomThreadHold:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.bloomStrength, "BloomStrength:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.bloomUpsampleFactor, "BloomUpsampleFactor:");

			JFileIOHelper::LoadAtomicData(tool, postProcess.hdrPaperWhite, "HdrPaperWhite:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.maxBrightness, "MaxBrightness:");

			JFileIOHelper::LoadAtomicData(tool, postProcess.fxaaContrastThreshold, "FxaaContrastThreshold:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.fxaaSubPixelRemoval, "FxaaSubPixelRemoval:");

			JFileIOHelper::LoadAtomicData(tool, postProcess.minExposure, "MinExposure:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.maxExposure, "MaxExposure:");

			JFileIOHelper::LoadAtomicData(tool, postProcess.targetLuminance, "TargetLuminance:");
			JFileIOHelper::LoadAtomicData(tool, postProcess.adaptationRate, "AdaptationRate:");

			tool.PopStack();
			tool.Close(); 
		}
		void JGraphicOption::Store()
		{
			JFileIOTool tool;
			const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
			if (!tool.Begin(path, JFileIOTool::TYPE::JSON))
				return;

			tool.PushMapMember("--Culling--");
			JFileIOHelper::StoreAtomicData(tool, culling.isOcclusionQueryActivated, "IsOcclusionQueryActivated:");
			JFileIOHelper::StoreAtomicData(tool, culling.isLightCullingActivated, "IsLightCullingActivated:");
			JFileIOHelper::StoreAtomicData(tool, culling.allowHZBCorrectFail, "AllowHZBCorrectFail:");
			JFileIOHelper::StoreAtomicData(tool, culling.allowHDOcclusionUseOccluder, "AllowHDOcclusionUseOccluder:");
			JFileIOHelper::StoreAtomicData(tool, culling.allowLightCluster, "AllowLightCluster:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterXIndex, "ClusterXIndex:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterYIndex, "ClusterYIndex:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterZIndex, "ClusterZIndex:");
			JFileIOHelper::StoreAtomicData(tool, culling.lightPerClusterIndex, "LightPerClusterIndex:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterNear, "ClusterNear:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterPointLightRangeOffset, "ClusterPointLightRangeOffset:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterSpotLightRangeOffset, "ClusterSpotLightRangeOffset:");
			JFileIOHelper::StoreAtomicData(tool, culling.clusterRectLightRangeOffset, "ClusterRectLightRangeOffset:");
			tool.PopStack();

			tool.PushMapMember("--Rendering--");
			JFileIOHelper::StoreAtomicData(tool, rendering.allowMultiThread, "AllowMultiThread:");
			JFileIOHelper::StoreAtomicData(tool, rendering.allowDrawGui, "AllowDrawGui:");
			JFileIOHelper::StoreAtomicData(tool, rendering.allowDeferred, "AllowDeferred:");
			JFileIOHelper::StoreAtomicData(tool, rendering.allowRaytracing, "AllowRaytracing:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useMSAA, "Msaa:");
			JFileIOHelper::StoreEnumData(tool, rendering.renderTargetFormat, "RenderTargetFormat:");

			JFileIOHelper::StoreAtomicData(tool, rendering.useGGXMicrofacet, "UseGGXMicrofacet:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useBeckmannMicrofacet, "UseBeckmannMicrofacet:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useBlinnPhongMicrofacet, "UseBlinnPhongMicrofacet:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useIsotropy, "UseIsotropy:");

			JFileIOHelper::StoreAtomicData(tool, rendering.useDisneyDiffuse, "UseDisneyDiffuse:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useFrostBiteDiffuse, "UseFrostBiteDiffuse:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useHammonDiffuse, "UseHammonDiffuse:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useOrenNayarDiffuse, "UseOrenNayarDiffuse:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useShirleyDiffuse, "UseShirleyDiffuse:");
			JFileIOHelper::StoreAtomicData(tool, rendering.useLambertianDiffuse, "UseLambertianDiffuse:");

			JFileIOHelper::StoreAtomicData(tool, rendering.restir.bounceCount.Get(), "RestirBounceCount:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.temporalSampleCount.Get(), "RestirTemporalSampleCount:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.spatialSampleCount.Get(), "RestirSpatialSampleCount:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.sampleMaxAge.Get(), "RestirSampleMaxAge:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.neighborWeightSumThreshold.Get(), "RestirNeighborWeightSumThreshold:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.spatialWeightAverageThreshold.Get(), "RestirSpatialWeightAverageThreshold:");
			JFileIOHelper::StoreAtomicData(tool, rendering.restir.useDenoiser, "ResitrUseDenoiser:");

			tool.PopStack();

			tool.PushMapMember("--Shadow--");
			JFileIOHelper::StoreAtomicData(tool, shadow.useHighQualityShadow, "UseHighQualityShadow:");
			JFileIOHelper::StoreAtomicData(tool, shadow.useMiddleQualityShadow, "UseMiddleQualityShadow:");
			JFileIOHelper::StoreAtomicData(tool, shadow.useLowQualityShadow, "UseLowQualityShadow:");
			tool.PopStack();

			tool.PushMapMember("--PostProcess--");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useFxaa, "UseFxaa:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useSsao, "UseSsao:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useSsaoInterleave, "UseSsaoInterleave:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.usePostprocess, "UsePostprocess:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useHdr, "UseHdr:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useToneMapping, "UseToneMapping:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useBloom, "UseBloom:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.useHistogramDebug, "UseHistogramDebug:");

			JFileIOHelper::StoreEnumData(tool, postProcess.exposureType, "ExposureType:");
			JFileIOHelper::StoreEnumData(tool, postProcess.bloomQuality, "BloomQuality:");

			JFileIOHelper::StoreAtomicData(tool, postProcess.bloomThreadHold, "BloomThreadHold:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.bloomStrength, "BloomStrength:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.bloomUpsampleFactor, "BloomUpsampleFactor:");

			JFileIOHelper::StoreAtomicData(tool, postProcess.hdrPaperWhite, "HdrPaperWhite:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.maxBrightness, "MaxBrightness:");

			JFileIOHelper::StoreAtomicData(tool, postProcess.fxaaContrastThreshold, "FxaaContrastThreshold:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.fxaaSubPixelRemoval, "FxaaSubPixelRemoval:");

			JFileIOHelper::StoreAtomicData(tool, postProcess.minExposure, "MinExposure:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.maxExposure, "MaxExposure:");

			JFileIOHelper::StoreAtomicData(tool, postProcess.targetLuminance, "TargetLuminance:");
			JFileIOHelper::StoreAtomicData(tool, postProcess.adaptationRate, "AdaptationRate:");

			tool.PopStack(); 
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			//Debugging data는 저장하지 않는다.
		}
	}
}