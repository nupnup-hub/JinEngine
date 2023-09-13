#pragma once
#include"../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo
		{
		public:
			static constexpr uint minCapacity = 4;
		public:
			static constexpr uint initBindTextureCapacity = 32;
			static constexpr uint initSkyTextureCapacity = 4;
			static constexpr uint initBindTShadowCapacity = 4;
		public:
			//FrameResource Count
			uint upObjCount = 0;
			uint upAniCount = 0;
			uint upEnginePassCount = 0;
			uint upScenePassCount = 0;
			uint upCameraCount = 0;
			uint updLightCount = 0;  
			uint uppLightCount = 0;
			uint upsLightCount = 0;
			uint upCsmCount = 0;
			uint upCubeShadowMapCount = 0;
			uint upNormalShadowMapCount = 0;
			uint upMaterialCount = 0;

			//FrameResource Capacity
			uint upObjCapacity = minCapacity;
			uint upAniCapacity = minCapacity;
			uint upEnginePassCapacity = minCapacity;
			uint upScenePassCapacity = minCapacity;
			uint upCameraCapacity = minCapacity;
			uint upLightCapacity = minCapacity;
			uint upSmLightCapacity = minCapacity;
			uint upMaterialCapacity = minCapacity;
		public:
			uint binding2DTextureCount = 0;
			uint bindingCubeMapCount = 0;
			uint bindingShadowTextureCount = 0;
			uint bindingShadowTextureArrayCount = 0;
			uint bindingShadowTextureCubeCount = 0;
		public:
			uint binding2DTextureCapacity = initBindTextureCapacity;
			uint bindingCubeMapCapacity = initSkyTextureCapacity;
			uint bindingShadowTextureCapacity = initBindTShadowCapacity;
			uint bindingShadowTextureArrayCapacity = initBindTShadowCapacity;
			uint bindingShadowTextureCubeCapacity = initBindTShadowCapacity;
		public:
			int width = 0;
			int height = 0;
		public:
			uint occlusionWidth = 0;
			uint occlusionHeight = 0;
			uint occlusionMinSize = 0;
			uint occlusionMapCount = 0;
			uint occlusionMapCapacity = 0;
		public:
			uint frameThreadCount = 0;
		};
	}
}