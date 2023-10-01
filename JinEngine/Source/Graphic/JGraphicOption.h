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
			bool useDirectionalLightPcss = true;
			bool usePointLightPcm = true;
			bool useSpotLightPcm = true;
		public:
			//Test option
			bool allowHdDistTest = false;
		public:
			J_GRAPHIC_DEVICE_TYPE deviceType = J_GRAPHIC_DEVICE_TYPE::DX12;
		public:
			bool IsOcclusionActivated()const noexcept;
			bool IsHDOccActivated()const noexcept;
			bool IsHZBOccActivated()const noexcept;
		};
	}
}