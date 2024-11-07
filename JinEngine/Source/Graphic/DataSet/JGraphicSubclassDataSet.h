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
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{
		/*
		* Subclass classification by process
		*/
		class JGraphicSubClassInterface;
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JCullingManager;
		class JGpuAcceleratorManager;
		class JCsmManager;
		class JFrameResourceManager;
		class JCommandContextManager;
		class JGraphicResourceShareData; 
		class JGraphicObjectDataSetManager;
		class JFrameResource;

		struct JGraphicInfo;
		struct JResourceManageSubclassSet
		{ 
		public:
			std::unique_ptr<JGraphicResourceManager> graphic;
			std::unique_ptr<JCullingManager> culling;
			std::unique_ptr<JCsmManager> csm;	//safe change device
			std::unique_ptr<JGpuAcceleratorManager> accelerator;
			std::unique_ptr<JFrameResourceManager> frame;
			std::unique_ptr<JGraphicResourceShareData> shareData; 
			std::unique_ptr<JGraphicObjectDataSetManager> objectData;
		public:
			std::unique_ptr<JCommandContextManager> context;
		public:
			//std::unique_ptr<JFrameResource> frame[Constants::gNumFrameResources];
			//JFrameResource* currFrame = nullptr;
		public:
			void Initialize(JGraphicDevice* device, const JGraphicInfo& info);
			void Clear();
		public:
			template<typename BaseClass>
			void Push(std::vector<BaseClass*>& outV)
			{
				if constexpr (std::is_base_of_v <BaseClass, JGraphicResourceManager>)
					outV.push_back(graphic.get());
				if constexpr (std::is_base_of_v <BaseClass, JCullingManager>)
					outV.push_back(culling.get());
				if constexpr (std::is_base_of_v <BaseClass, JCsmManager>)
					outV.push_back(csm.get());
				if constexpr (std::is_base_of_v <BaseClass, JGpuAcceleratorManager>)
					outV.push_back(accelerator.get());

				if constexpr (std::is_base_of_v <BaseClass, JFrameResourceManager>)
					outV.push_back(frame.get());
				if constexpr (std::is_base_of_v <BaseClass, JGraphicResourceShareData>)
					outV.push_back(shareData.get());
				if constexpr (std::is_base_of_v <BaseClass, JGraphicObjectDataSetManager>)
					outV.push_back(objectData.get());
				if constexpr (std::is_base_of_v <BaseClass, JCommandContextManager>)
					outV.push_back(context.get());
			}
		};

		class JSceneDraw;
		class JSceneDependencyData;
		class JShadowMap;
		class JDepthTest; 
		class JOutline;
		class JGraphicDebug;

		//related to scene object drawing
		struct JSceneDrawingSubclassSet
		{
		public:
			//Drawing scene object
			std::unique_ptr<JSceneDraw> scene;
			std::unique_ptr<JShadowMap> shadowMap;
			std::unique_ptr<JDepthTest> depthTest;
		public:
			//Compute scene task
			std::unique_ptr<JOutline> outline;
			std::unique_ptr<JGraphicDebug> debug;
			std::unique_ptr<JSceneDependencyData> sceneDependencyData;
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear();
		public:
			template<typename BaseClass>
			void Push(std::vector<BaseClass*>& outV)
			{
				if constexpr (std::is_base_of_v <BaseClass, JSceneDraw>)
					outV.push_back(scene.get());
				if constexpr (std::is_base_of_v <BaseClass, JShadowMap>)
					outV.push_back(shadowMap.get());
				if constexpr (std::is_base_of_v <BaseClass, JDepthTest>)
					outV.push_back(depthTest.get());
				if constexpr (std::is_base_of_v <BaseClass, JOutline>)
					outV.push_back(outline.get());
				if constexpr (std::is_base_of_v <BaseClass, JGraphicDebug>)
					outV.push_back(debug.get());
				if constexpr (std::is_base_of_v <BaseClass, JSceneDependencyData>)
					outV.push_back(sceneDependencyData.get());
			}
		};


		class JFrustumCulling;
		class JHardwareOccCulling;
		class JHZBOccCulling;
		class JLightCulling; 

		//related to culling
		struct JCullingSubclassSet
		{
		public:
			std::unique_ptr<JFrustumCulling> frustum;
			std::unique_ptr<JHardwareOccCulling> hd;
			std::unique_ptr<JHZBOccCulling> hzb;
			std::unique_ptr<JLightCulling> lit;
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear();
		public:
			template<typename BaseClass>
			void Push(std::vector<BaseClass*>& outV)
			{
				if constexpr (std::is_base_of_v <BaseClass, JFrustumCulling>)
					outV.push_back(frustum.get());
				if constexpr (std::is_base_of_v <BaseClass, JHardwareOccCulling>)
					outV.push_back(hd.get());
				if constexpr (std::is_base_of_v <BaseClass, JHZBOccCulling>)
					outV.push_back(hzb.get());
				if constexpr (std::is_base_of_v <BaseClass, JLightCulling>)
					outV.push_back(lit.get());
			}
		};
		 
		class JBlur;
		class JDownSampling;
		class JSsao;
		class JSsr;
		class JToneMapping;
		class JBloom;
		class JAntialise;
		class JPostProcessHistogram;
		class JPostProcessExposure;
		class JConvertColor;
		class JPostProcessPipeline; 
		struct JPostProcessEffectSet;

		//related to image processing
		struct JImageProcessingSubclassSet
		{
		public:
			//post process
			std::unique_ptr<JBlur> blur;
			std::unique_ptr<JDownSampling> downSampling;
			std::unique_ptr<JSsao> ssao;
			std::unique_ptr<JSsr> ssr;
			std::unique_ptr<JToneMapping> tm;
			std::unique_ptr<JBloom> bloom;
			std::unique_ptr<JAntialise> aa;
			std::unique_ptr<JPostProcessHistogram> histogram;
			std::unique_ptr<JPostProcessExposure> exposure;
			std::unique_ptr<JConvertColor> convertColor;
			std::unique_ptr<JPostProcessEffectSet> ppEffectSet;			//data set
			std::unique_ptr<JPostProcessPipeline> ppPipeline;			//pipe line 
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear(); 
		public:
			template<typename BaseClass>
			void Push(std::vector<BaseClass*>& outV)
			{
				if constexpr (std::is_base_of_v <BaseClass, JBlur>)
					outV.push_back(blur.get());
				if constexpr (std::is_base_of_v <BaseClass, JDownSampling>)
					outV.push_back(downSampling.get());
				if constexpr (std::is_base_of_v <BaseClass, JSsao>)
					outV.push_back(ssao.get());
				if constexpr (std::is_base_of_v <BaseClass, JSsr>)
					outV.push_back(ssr.get());

				if constexpr (std::is_base_of_v <BaseClass, JToneMapping>)
					outV.push_back(tm.get());
				if constexpr (std::is_base_of_v <BaseClass, JBloom>)
					outV.push_back(bloom.get());
				if constexpr (std::is_base_of_v <BaseClass, JAntialise>)
					outV.push_back(aa.get());
				if constexpr (std::is_base_of_v <BaseClass, JPostProcessHistogram>)
					outV.push_back(histogram.get());

				if constexpr (std::is_base_of_v <BaseClass, JPostProcessExposure>)
					outV.push_back(exposure.get());
				if constexpr (std::is_base_of_v <BaseClass, JConvertColor>)
					outV.push_back(convertColor.get());
				if constexpr (std::is_base_of_v <BaseClass, JPostProcessEffectSet>)
					outV.push_back(ppEffectSet.get());
				if constexpr (std::is_base_of_v <BaseClass, JPostProcessPipeline>)
					outV.push_back(ppPipeline.get());
			}
		};
		  
		class JRaytracingGI;
		class JRaytracingAmbientOcclusion;
		class JRaytracingDenoiser;

		//related to raytracing
		struct JRaytracingSubclassSet
		{
		public:
			std::unique_ptr<JRaytracingGI> gi;
			std::unique_ptr<JRaytracingAmbientOcclusion> ao;  
			std::unique_ptr<JRaytracingDenoiser> denoiser;
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear();
		public:
			template<typename BaseClass>
			void Push(std::vector<BaseClass*>& outV)
			{
				if constexpr (std::is_base_of_v <BaseClass, JRaytracingGI>)
					outV.push_back(gi.get());
				if constexpr (std::is_base_of_v <BaseClass, JRaytracingAmbientOcclusion>)
					outV.push_back(ao.get());
				if constexpr (std::is_base_of_v <BaseClass, JRaytracingDenoiser>)
					outV.push_back(denoiser.get());
			}
		};
	}
}