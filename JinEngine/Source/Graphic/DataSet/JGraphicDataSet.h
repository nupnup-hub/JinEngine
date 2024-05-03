#pragma once
#include"JGraphicSubclassDataSet.h"
#include"../JGraphicConstants.h"
#include"../JGraphicInfo.h"
#include"../JGraphicOption.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../Image/JImageProcessingDesc.h" 
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Handle/JDataHandleStructure.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		//Draw = Graphic shader
		//Compute = compute shader
		//Task = Draw & Compute

		/**
		* Wrapping class for passing for draw
		*/
		/**
		* DataSet은 API마다 사용되는 객체를 전방선언한다.
		*/   
		class JCullingUserInterface; 
		class JShareDataHolderInterface;
		class JCommandContext;

		struct JGraphicDeviceInitSet
		{ 
		public:
			JGraphicResourceManager* graphicResourceM;
		public:
			JGraphicDeviceInitSet(JGraphicResourceManager* graphicResourceM);
		};

		struct JGraphicGuiInitSet
		{
		public:
			JGraphicResourceManager* graphicResourceM;
			JGraphicDevice* graphicDevice;
		}; 	 

		struct JPostProcessEffectSet : public Core::JValidInterface
		{
		public:
			JToneMapping* tm;
			JBloom* bloom;
			JAntialise* aa;
			JPostProcessHistogram* histogram;
			JPostProcessExposure* exposure;
			JConvertColor* convertColor;
		public:
			JPostProcessEffectSet(JToneMapping* tm,
				JBloom* bloom,
				JAntialise* aa,
				JPostProcessHistogram* histogram,
				JPostProcessExposure* exposure,
				JConvertColor* convertColor);
		};

		struct JGraphicDrawReferenceSet : public Core::JValidInterface
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
			JGpuAcceleratorManager* acceleratorM;
		public:
			JFrameResource* currFrame;
		public:
			JGraphicDebug* depthDebug;
			JDepthTest* depthTest;
		public: 
			JBlur* blur;
			JDownSampling* downSample;
			JSsao* ssao; 
			JPostProcessEffectSet* postSet;
		public:
			JGraphicResourceShareData* shareData;
		public:
			const int currFrameIndex;
			const int nextFrameIndex;
		public:
			JGraphicDrawReferenceSet(const JGraphicInfo& info,
				const JGraphicOption& option,
				JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JGpuAcceleratorManager* acceleratorM,
				JFrameResource* currFrame,
				JGraphicDebug* depthDebug,
				JDepthTest* depthTest,
				JBlur* blur,
				JDownSampling* downSample,
				JSsao* ssao, 
				JPostProcessEffectSet* postSet,
				JGraphicResourceShareData* shareData,
				const int currFrameIndex,
				const int nextFrameIndex); 
		};

		struct JGraphicShaderCompileSet
		{
		public:
			JGraphicDevice* device; 
		public:
			JGraphicShaderCompileSet(JGraphicDevice* device); 
		};
 
		//device type per draw data set 
		struct JGraphicBindSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicBindSet(JCommandContext* context);
		};

		struct JGraphicOccDrawSet;
		struct JGraphicDepthMapDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicDepthMapDrawSet(JCommandContext* context);
			JGraphicDepthMapDrawSet(const JGraphicOccDrawSet* drawSet);
		};

		struct JGraphicDebugRsComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicDebugRsComputeSet(JCommandContext* context);
		};

		struct JGraphicSceneDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicSceneDrawSet(JCommandContext* context);
		};

		struct JGraphicShadowMapDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
			JBlur* blur;
		public:
			JGraphicShadowMapDrawSet(JCommandContext* context, JBlur* blur);
		};

		struct JGraphicOccDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
			JDepthTest* depthTest;
		public:
			JGraphicOccDrawSet(JCommandContext* context, JDepthTest* depthTest);
		};

		struct JGraphicHzbOccComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicHzbOccComputeSet(JCommandContext* context);
		};

		struct JGraphicHdOccExtractSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicHdOccExtractSet(JCommandContext* context);
		};

		struct JGraphicOutlineDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicOutlineDrawSet(JCommandContext* context);
		};

		struct JGraphicBlurComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context; 
			std::unique_ptr<JBlurDesc> desc;
		public:
			JGraphicBlurComputeSet(JCommandContext* context, std::unique_ptr<JBlurDesc>&& desc);
		};

		struct JGraphicDownSampleComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
			JBlur* blur;
			std::unique_ptr<JDownSampleDesc> desc;
		public:
			const std::vector<Core::JDataHandle>& handle;
		public:
			JGraphicDownSampleComputeSet(JCommandContext* context,
				JBlur* blur,
				std::unique_ptr<JDownSampleDesc>&& desc,
				const std::vector<Core::JDataHandle>& handle);
		};

		struct JGraphicSsaoComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
			JGraphicResourceShareData* shareData;
		public:
			JGraphicSsaoComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData);
		};

		struct JGraphicAAComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
			JGraphicResourceShareData* shareData;
		public:
			JGraphicAAComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData);
		};

		//추후에 color curve를 변경하는 기능 추가
		struct JGraphicConvertColorComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{ 
		public:
			JCommandContext* context; 
		public:
			JConvertColorDesc desc;
		public:
			JGraphicConvertColorComputeSet(JCommandContext* context, const JConvertColorDesc& desc);
		};

		struct JPostProcessComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JPostProcessEffectSet* ppSet;
		public:
			JCommandContext* context;
			JGraphicResourceManager* gm;
			JGraphicResourceShareData* shareData; 
		public:
			JShareDataHolderInterface* imageShareData;
		public:
			JPostProcessComputeSet(JPostProcessEffectSet* ppSet, JCommandContext* context, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData);
		};

		struct JGraphicLightCullingTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicLightCullingTaskSet(JCommandContext* context);
		};

		struct JGraphicLightCullingDebugDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicLightCullingDebugDrawSet(JCommandContext* context);
		};

		struct JGraphicRtAoComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicRtAoComputeSet(JCommandContext* context);
		};

		struct JGraphicRtGiComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context; 
		public:
			JGraphicRtGiComputeSet(JCommandContext* context);
		};
		 
		struct JGraphicRtDenoiseComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCommandContext* context;
		public:
			JGraphicRtDenoiseComputeSet(JCommandContext* context);
		};

		//draw scene single thread
		struct JGraphicDrawSceneSTSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicSceneDrawSet> sceneDraw;
			std::unique_ptr<JGraphicShadowMapDrawSet> shadowMapDraw;
			std::unique_ptr<JGraphicOccDrawSet> occDraw;
			std::unique_ptr<JGraphicHzbOccComputeSet> hzbCompute;
			std::unique_ptr<JGraphicHdOccExtractSet> hdExtract;
			std::unique_ptr<JGraphicDebugRsComputeSet> debugCompute;
			std::unique_ptr<JGraphicOutlineDrawSet> outline;
			std::unique_ptr<JGraphicSsaoComputeSet> ssao; 
			std::unique_ptr<JPostProcessComputeSet> postPrcess;
			std::unique_ptr<JGraphicLightCullingTaskSet> litCulling;
			std::unique_ptr<JGraphicLightCullingDebugDrawSet> litCullingDebug;
			std::unique_ptr<JGraphicRtAoComputeSet> rtao;
			std::unique_ptr<JGraphicRtGiComputeSet> rtgi; 
			std::unique_ptr<JGraphicRtDenoiseComputeSet> rtDenoiser;
		};

		struct JGraphicBeginFrameSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicLightCullingTaskSet> litCulling;
		};

		struct JGraphicMidFrameSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicSceneDrawSet> sceneDraw;
			std::unique_ptr<JGraphicHzbOccComputeSet> hzbCompute;
			std::unique_ptr<JGraphicHdOccExtractSet> hdExtract;
			std::unique_ptr<JGraphicDebugRsComputeSet> debugCompute;
			std::unique_ptr<JGraphicOutlineDrawSet> outline;
			std::unique_ptr<JGraphicSsaoComputeSet> ssao; 
			std::unique_ptr<JPostProcessComputeSet> postPrcess;
			std::unique_ptr<JGraphicLightCullingDebugDrawSet> litCullingDebug;
			std::unique_ptr<JGraphicRtAoComputeSet> rtao;
			std::unique_ptr<JGraphicRtGiComputeSet> rtgi; 
			std::unique_ptr<JGraphicRtDenoiseComputeSet> rtDenoiser;
		};

		struct JGraphicEndConditonSet
		{
		public:
			bool isSceneDrawn = true;
		public:
			JGraphicEndConditonSet(const bool isSceneDrawn);
		};

		struct JGraphicThreadOccTaskSet
		{
		public:
			std::unique_ptr<JGraphicOccDrawSet> occDraw;
		};

		struct JGraphicThreadShadowMapTaskSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicShadowMapDrawSet> shadowMapDraw;
		};

		struct JGraphicThreadSceneTaskSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicOccDrawSet> occDraw; //hdd is called after scene draw so it is use scene cmd
			std::unique_ptr<JGraphicSceneDrawSet> sceneDraw;
		};
 
		using PushGraphicEventPtr = void(*)(std::unique_ptr<Core::JBindHandleBase>&&);
		class JFrameIndexAccess;
		//can refer any subclass
		struct JGraphicSubClassShareData
		{  
		public:
			JFrameIndexAccess* frameIndexAccess;
			PushGraphicEventPtr pushGraphicEventPtr;
		public:
			JGraphicSubClassShareData(JFrameIndexAccess* frameIndexAccess, PushGraphicEventPtr pushGraphicEventPtr);
		};

		struct JGraphicInfoChangedSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			const JGraphicInfo& preInfo;
			const JGraphicInfo& newInfo;
			JGraphicInfo::TYPE changedPart;
		public:
			JGraphicInfoChangedSet(const JGraphicInfo& preInfo, const JGraphicInfo& newInfo);
		};
		struct JGraphicOptionChangedSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			const JGraphicOption& preOption;
			const JGraphicOption& newOption;
			JGraphicOption::TYPE changedPart;
		public:
			JGraphicOptionChangedSet(const JGraphicOption& preOption, const JGraphicOption& newOption);
		};
	}
}