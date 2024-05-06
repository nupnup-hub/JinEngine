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


#pragma once
#include"../Core/JCoreEssential.h"  

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo
		{ 
		public:
			enum class TYPE
			{
				FRAME,
				RESOURCE,
				COUNT
			};
		public:
			static constexpr uint minCapacity = 4;
		public:
			static constexpr uint initBindTextureCapacity = 32;
			static constexpr uint initSkyTextureCapacity = 4;
			static constexpr uint initBindTShadowCapacity = 4;
		public:
			struct FrameResourceInfo
			{
			public:
				//FrameResource Count
				uint upObjCount = 0;
				uint upBoundingObjCount = 0;
				uint upHzbObjCount = 0;
				uint upAniCount = 0;
				uint upScenePassCount = 0;
				uint upCameraCount = 0;
				uint upDLightCount = 0;
				uint upPLightCount = 0;
				uint upSLightCount = 0;
				uint upRLightCount = 0;
				uint upCsmCount = 0;
				uint upCubeShadowMapCount = 0;
				uint upNormalShadowMapCount = 0;
				uint upMaterialCount = 0;
			public:
				//FrameResource Capacity
				uint upObjCapacity = minCapacity;
				uint upBoundingObjCapacity = minCapacity;
				uint upHzbObjCapacity = minCapacity;
				uint upAniCapacity = minCapacity;
				uint upScenePassCapacity = minCapacity;
				uint upCameraCapacity = minCapacity;
				uint upDLightCapacity = minCapacity;
				uint upPLightCapacity = minCapacity;
				uint upSLightCapacity = minCapacity;
				uint upRLightCapacity = minCapacity;
				uint upSmLightCapacity = minCapacity;
				uint upMaterialCapacity = minCapacity;
			public:
				uint threadCount = 0;
			public:
				int currIndex = 0;
			public:
				uint GetLocalLightCapacity()const noexcept;
			};
			struct GraphicResourceInfo
			{
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
				uint occlusionWidth = 0;
				uint occlusionHeight = 0;
				uint occlusionMinSize = 0;
				uint occlusionMapCount = 0;
				uint occlusionMapCapacity = 0;
			};
		public:
			FrameResourceInfo frame;
			GraphicResourceInfo resource; 
		public:
			//window size
			int width = 0;
			int height = 0;  
		public:
			void Load();
			void Store();
		};
	}
}