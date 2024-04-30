#pragma once 
#include"Device/JGraphicDeviceType.h"
#include"Culling/JCullingConstants.h"
#include"Image/JImageProcessingEnum.h"
#include"JGraphicConstants.h"
#include"../Core/JCoreEssential.h"
#include"../Core/Unit/JManagedVariable.h"
#include"JGraphicEnum.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JGraphicOption
		{
		public:
			enum class TYPE
			{
				CULLING,
				RENDERING,
				SHAODW,
				POST_PROCESS,
				DEBUGGING,
				COUNT
			};
		public:
			struct Culling
			{
			public: 
				bool isOcclusionQueryActivated = true;
				bool isLightCullingActivated = true;
				bool allowHZBCorrectFail = true;
				bool allowHDOcclusionUseOccluder = false;
				bool allowLightCluster = false;
			public:
				//Constants::Cluster range array index
				uint clusterXIndex = 0;
				uint clusterYIndex = 0;
				uint clusterZIndex = 0;
				uint lightPerClusterIndex = 1;
				float clusterNear = Constants::litClusterNear;
				float clusterPointLightRangeOffset = Constants::litClusterRangeOffset;
				float clusterSpotLightRangeOffset = Constants::litClusterRangeOffset;
				float clusterRectLightRangeOffset = Constants::litClusterRangeOffset; 
			public: 
				bool LightCullingDependencyChanged(const JGraphicOption& rhs)const noexcept;
			}; 
			struct Rendering
			{
			public:
				struct Restir
				{
				public:
					Core::JRestrictedConstRangeVar<int> temporalSampleCount = Core::JRestrictedConstRangeVar<int>(Constants::restirTemporalSampleRange.CreateConstRangeVar<int>());
					Core::JRestrictedConstRangeVar<int> spatialSampleCount = Core::JRestrictedConstRangeVar<int>(Constants::restirSpatialSampleRange.CreateConstRangeVar<int>());
					Core::JRestrictedConstRangeVar<int> sampleMaxAge = Core::JRestrictedConstRangeVar<int>(Constants::restirSampleAgeRange.CreateConstRangeVar<int>());
					Core::JRestrictedConstRangeVar<float> neighborWeightSumThreshold = Core::JRestrictedConstRangeVar<float>(Constants::neighborWeightSumThresholdRange.CreateConstRangeVar<float>());
					Core::JRestrictedConstRangeVar<float> spatialWeightAverageThreshold = Core::JRestrictedConstRangeVar<float>(Constants::spatialWeightAverageThresholdRange.CreateConstRangeVar<float>());
				public:
					bool operator==(const Restir& rhs)const noexcept;
					bool operator!=(const Restir& rhs)const noexcept;
				};
			public:
				//Rendering
				bool allowMultiThread = true;
				bool allowDrawGui = true;
				bool allowDeferred = false;
				bool allowRaytracing = false;
			public:
				bool useMSAA = false;			//deferred에서 미구현
			public:
				J_GRAPHIC_FORMAT renderTargetFormat = J_GRAPHIC_FORMAT::R8G8B8A8_UNORM;
			public:
				//BRDF ndf
				bool useGGXMicrofacet = true;
				bool useBeckmannMicrofacet = false;
				bool useBlinnPhongMicrofacet = false;
				bool useIsotropy = true;
			public:
				//BRDF diffuse
				bool useDisneyDiffuse = true;
				bool useFrostBiteDiffuse = false;
				bool useHammonDiffuse = false;
				bool useOrenNayarDiffuse = false;
				bool useShirleyDiffuse = false;
				bool useLambertianDiffuse = false; 
			public:
				//global light
				Restir restir;
			};
			struct Shadow
			{
			public:
				//Shadow option
				bool useHighQualityShadow = true; //dLit pcss other light pcf 32
				bool useMiddleQualityShadow = false; //all light pcf 32
				bool useLowQualityShadow = false;	//all light pcf 16
			};
			struct PostProcess
			{
			public:
				bool useFxaa = false;
				bool useSsao = false;
				bool useSsaoInterleave = true;
				bool usePostprocess = false;
				bool useHdr = false;
			public:
				//Test Option
				//tone mapping을 비롯한 post processing option은
				//추후 post processing volume component로 이식된다
				//현재는 test에 편의를 위해 graphic option에서 설정한다.
				bool useToneMapping = false;
				bool useBloom = false;
				bool useHistogramDebug = false;
				J_EXPOSURE_TYPE exposureType = J_EXPOSURE_TYPE::NONE;
				J_BLOOM_QUALITY bloomQuality = J_BLOOM_QUALITY::HIGH;
				float bloomThreadHold = 4.0f;		// The threshold luminance above which a pixel will start to bloom
				float bloomStrength = 0.1f;			// A modulator controlling how much bloom is added back into the image
				float bloomUpsampleFactor = 0.65f;			// Controls the "focus" of the blur.  High values spread out more causing a haze.
				float hdrPaperWhite = 200;			// PaperWhite / MaxBrightness
				float maxBrightness = 1000;
				float fxaaContrastThreshold = 0.175f;
				float fxaaSubPixelRemoval = 0.5f;
				float minExposure = -8.0f;
				float maxExposure = 8.0f;
				float targetLuminance = 0.08f;
				float adaptationRate = 0.05f; 
			}; 
			struct Debugging
			{
			public:
				//Debugging
				bool allowOutline = true;
				bool allowDisplayLightCullingResult = false;
#ifdef DEVELOP
				//bool testClearLitCullingOffsetBuffer = false;
				bool requestRecompileGraphicShader = false;			//for debugging
				bool requestRecompileLightClusterShader = false;	//for debugging
				bool requestRecompileSsaoShader = false;			//for debugging 
				bool requestRecompileToneMappingShader = false;		//for debugging
				bool requestRecompileRtGiShader = false;			//for debugging 
				bool requestRecompileRtDenoiseShader = false;		//for debugging 
				bool testTrigger00 = false;
				bool testTrigger01 = false;
				bool testTrigger02 = false;
				//bool drawSsaoByComputeShader = true;
			//---------------------------------------------------------
#endif
			};
		public:
			Culling culling; 
			Rendering rendering;  
			Shadow shadow;
			PostProcess postProcess;
			Debugging debugging;
		public:
			J_GRAPHIC_DEVICE_TYPE deviceType = J_GRAPHIC_DEVICE_TYPE::DX12;
		public:
			uint GetClusterTotalCount()const noexcept;
			uint GetClusterIndexCount()const noexcept;
			uint GetClusterXCount()const noexcept;
			uint GetClusterYCount()const noexcept;
			uint GetClusterZCount()const noexcept;
			uint GetLightPerCluster()const noexcept;
		public:
			bool IsOcclusionActivated()const noexcept;  
			bool IsPostProcessActivated()const noexcept;
			bool CanUseRtGi()const noexcept;
			bool CanUsePoseProcess()const noexcept;
			bool CanUseSSAO()const noexcept; 
		public:
			void Load();
			void Store();
		};	   
	}
}