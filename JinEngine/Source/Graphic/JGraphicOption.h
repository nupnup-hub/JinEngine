#pragma once 
#include"Device/JGraphicDeviceType.h"
#include"../Core/JCoreEssential.h"
#include"Culling/JCullingConstants.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JGraphicOption
		{
		public:
			//culing
			bool isOcclusionQueryActivated = true; 
			//bool isHDOcclusionAcitvated = false;
			//bool isHZBOcclusionActivated = false; 
			bool isLightCullingActivated = true;
		public:
			bool allowHZBCorrectFail = true;
			bool allowHDOcclusionUseOccluder = false;
		public:
			//Debugging
			bool allowDebugOutline = true; 
			bool allowDebugLightCulling = false;
		public:
			//Rendering
			bool allowMultiThread = true;
			bool allowDrawGui = true; 
		public: 
			bool allowLightCluster = false;
			bool allowDeferred = false;
		public:
			bool useMSAA = false;
		public:
			//Shadow option
			bool useHighQualityShadow = true; //dLit pcss other light pcf 32
			bool useMiddleQualityShadow = false; //all light pcf 32
			bool useLowQualityShadow = false;	//all light pcf 16
		public:
			//ao
			bool useSsao = false;		//screen space ambient occlusion 
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
			//BRDF masking
			bool useSmithMasking = true;
			bool useTorranceMaskig = false;
		public:
			//BRDF ndf
			bool useGGXNDF = true;
			bool useBeckmannNDF = false;
			bool useBlinnPhongNDF = false;
			bool useIsotropy = true;
		public:
			//BRDF diffuse
			bool useDisneyDiffuse = true;
			bool useHammonDiffuse = false;
			bool useOrenNayarDiffuse = false;
			bool useShirleyDiffuse = false;
			bool useLambertianDiffuse = false;
		//Test option----------------------------------------------
		public:
#ifdef DEVELOP
			//bool testClearLitCullingOffsetBuffer = false;
			bool requestRecompileGraphicShader = false;			//for debugging
			bool requestRecompileLightClusterShader = false;	//for debugging
			bool requestRecompileSsaoShader = false;			//for debugging 
			bool useSsaoInterleave = false;
			//bool drawSsaoByComputeShader = true;
		//---------------------------------------------------------
#endif
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
			bool CanUseSSAO()const noexcept;
		};	   
	}
}