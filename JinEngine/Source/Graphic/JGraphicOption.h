#pragma once 
#include"Device/JGraphicDeviceType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicOption
		{
		public:
			bool isOcclusionQueryActivated = true; 
			bool isHDOcclusionAcitvated = false;
			bool isHZBOcclusionActivated = false; 
		public:
			bool allowHZBCorrectFail = true;
			bool allowDebugOutline = true;
		public:
			bool allowMultiThread = true;
			bool allowDrawGui = true; 
		public:
			//Shadow option
			bool useDirectionalLightPcm = false;
			bool useDirectionalLightPcmHighQuality = false;
			bool useDirectionalLightPcss = true; 
			bool usePointLightPcm = true; 
			bool usePointLightPcmHighQuality = false;
			bool usePointLightPcss = false;
			bool useSpotLightPcm = true; 
			bool useSpotLightPcmHighQuality = false;
			bool useSpotLightPcss = false;
		public:
//Test option----------------------------------------------
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
			bool useShirleyDiffuse = false;
			bool useLambertianDiffuse = false;
//---------------------------------------------------------
		public:
			J_GRAPHIC_DEVICE_TYPE deviceType = J_GRAPHIC_DEVICE_TYPE::DX12;
		public:
			bool IsOcclusionActivated()const noexcept;
			bool IsHDOccActivated()const noexcept;
			bool IsHZBOccActivated()const noexcept;
		};
	}
}