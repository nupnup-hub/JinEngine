#pragma once
#include"../Core/JDataType.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo
		{
		public:
			const uint minCapacity = 4;
		public:
			const uint initBindTextureCapacity = 32;
			const uint initSkyTextureCapacity = 4;
			const uint initBindTShadowCapacity = 4;
		public:
			//FrameResource Count
			uint upObjCount = 0;
			uint upAniCount = 0;
			uint upPassCount = 0;
			uint upCameraCount = 0;
			uint upLightCount = 0;
			uint upSmLightCount = 0;
			uint upMaterialCount = 0;

			//FrameResource Capacity
			uint upObjCapacity = minCapacity;
			uint upAniCapacity = minCapacity;
			uint upPassCapacity = minCapacity;
			uint upCameraCapacity = minCapacity;
			uint upLightCapacity = minCapacity;
			uint upSmLightCapacity = minCapacity;
			uint upMaterialCapacity = minCapacity;
		public:
			uint binding2DTextureCount = 0;
			uint bindingCubeMapCount = 0;
			uint bindingShadowTextureCount = 0;
		public:
			uint binding2DTextureCapacity = initBindTextureCapacity;
			uint bindingCubeMapCapacity = initSkyTextureCapacity;
			uint bindingShadowTextureCapacity = initBindTShadowCapacity;
		public:
			int width = 0;
			int height = 0;
			uint occlusionWidth = 0;
			uint occlusionHeight = 0;
			uint occlusionMinSize = 0;
			uint occlusionMapCount = 0;
			uint occlusionMapCapacity = 0;
			const uint defaultShadowWidth = 1920;
			const uint defaultShadowHeight = 1080;
		public:
			uint frameThreadCount = 0;
		};
	}
}