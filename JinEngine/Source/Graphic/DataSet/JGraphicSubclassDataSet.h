#pragma once
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h"
#include<memory>
#include<vector>
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicSubClassInterface;
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JCullingManager;
		class JGpuAcceleratorManager;
		class JCsmManager;
		class JCommandContextManager;
		class JGraphicResourceShareData; 
		class JFrameResource;

		struct JGraphicInfo;
		struct JResourceManageSubclassSet
		{ 
		public:
			std::unique_ptr<JGraphicResourceManager> graphic;
			std::unique_ptr<JCullingManager> culling;
			std::unique_ptr<JGpuAcceleratorManager> accelerator;
			std::unique_ptr<JCsmManager> csm;	//safe change device
			std::unique_ptr<JGraphicResourceShareData> shareData; 
		public:
			std::unique_ptr<JCommandContextManager> context;
		public:
			std::unique_ptr<JFrameResource> frame[Constants::gNumFrameResources];
			JFrameResource* currFrame = nullptr;
		public:
			void Initialize(JGraphicDevice* device, const JGraphicInfo& info);
			void Clear();
		public:
			void GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV);
		};

		class JSceneDraw;
		class JShadowMap;
		class JDepthTest;
		struct JDrawingSubclassSet
		{
		public:
			std::unique_ptr<JSceneDraw> scene;
			std::unique_ptr<JShadowMap> shadowMap;
			std::unique_ptr<JDepthTest> depthTest;
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear();
		public:
			void GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV);
		};


		class JFrustumCulling;
		class JHardwareOccCulling;
		class JHZBOccCulling;
		class JLightCulling;
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
			void GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV);
		};

		class JGraphicDebug;
		class JOutline;
		class JBlur;
		class JDownSampling;
		class JSsao;
		class JToneMapping;
		class JBloom;
		class JAntialise;
		class JPostProcessHistogram;
		class JPostProcessExposure;
		class JConvertColor;
		class JPostProcessPipeline; 
		struct JPostProcessEffectSet;
		struct JImageProcessingSubclassSet
		{
		public:
			//post process
			std::unique_ptr<JGraphicDebug> debug;
			std::unique_ptr<JOutline> outline;
			std::unique_ptr<JBlur> blur;
			std::unique_ptr<JDownSampling> downSampling;
			std::unique_ptr<JSsao> ssao;
			std::unique_ptr<JToneMapping> tm;
			std::unique_ptr<JBloom> bloom;
			std::unique_ptr<JAntialise> aa;
			std::unique_ptr<JPostProcessHistogram> histogram;
			std::unique_ptr<JPostProcessExposure> exposure;
			std::unique_ptr<JConvertColor> convertColor;
			std::unique_ptr<JPostProcessEffectSet> ppEffectSet;	//data set
			std::unique_ptr<JPostProcessPipeline> ppPipeline;			//pipe line 
		public:
			void Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage);
			void Clear();
		public:
			void GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV);
		};
		  
		class JRaytracingGI;
		class JRaytracingAmbientOcclusion;
		class JRaytracingDenoiser;
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
			void GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV);
		};
	}
}