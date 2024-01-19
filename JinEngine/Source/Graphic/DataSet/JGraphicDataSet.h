#pragma once
#include"../JGraphicInfo.h"
#include"../JGraphicOption.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../Image/JImageProcessingDesc.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Handle/JDataHandleStructure.h"
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
		class JGraphicResourceManager;
		class JCullingManager; 
		class JGraphicDevice;
		class JFrameResource;
		class JDepthTest;
		class JGraphicDebug;
		class JCullingUserInterface; 
		class JImageProcessing;

		struct JGraphicBaseDataSet
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JGraphicBaseDataSet(const JGraphicInfo& info, const JGraphicOption& option);
		};

		struct JGraphicDeviceInitSet
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JGraphicResourceManager* graphicResourceM;
		public:
			JGraphicDeviceInitSet(const JGraphicInfo& info, const JGraphicOption& option, JGraphicResourceManager* graphicResourceM);
		};

		struct JGraphicGuiInitSet
		{ 
		public:
			JGraphicResourceManager* graphicResourceM;
			JGraphicDevice* gDevice;
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
		public:
			JFrameResource* currFrame;
		public:
			JGraphicDebug* depthDebug;
			JDepthTest* depthTest;
		public:
			JImageProcessing* image;
		public:
			const int currFrameIndex;
			const int nextFrameIndex;
		public:
			JGraphicDrawReferenceSet(const JGraphicInfo& info, 
				const JGraphicOption& option,
				JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JFrameResource* currFrame,
				JGraphicDebug* depthDebug,
				JDepthTest* depthTest,
				JImageProcessing* image,
				const int currFrameIndex,
				const int nextFrameIndex);
		};

		struct JGraphicShaderCompileSet
		{
		public:
			JGraphicDevice* device;
			JGraphicBaseDataSet base; 
		public:
			JGraphicShaderCompileSet(JGraphicDevice* device, const JGraphicBaseDataSet& base);
			JGraphicShaderCompileSet(JGraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option);
		};

		//device type per draw data set 
		struct JGraphicBindSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JFrameResource* currFrame;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingManager;
		public:
			JGraphicBindSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingManager);
		};

		struct JGraphicOccDrawSet;
		struct JGraphicDepthMapDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public: 
			JFrameResource* currFrame; 
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
		public:
			JGraphicDepthMapDrawSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingM);
			JGraphicDepthMapDrawSet(const JGraphicOccDrawSet* drawSet);
		};
    
		struct JGraphicDebugRsComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device = nullptr;
			JGraphicResourceManager* graphicResourceM = nullptr;
		public:
			JGraphicDebugRsComputeSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM);
		};
		 
		struct JGraphicSceneDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JFrameResource* currFrame;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
		public:
			JGraphicSceneDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM);
		};

		struct JGraphicShadowMapDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JFrameResource* currFrame;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
			JImageProcessing* image;
		public:
			JGraphicShadowMapDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JImageProcessing* image);
		};

		struct JGraphicOccDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JFrameResource* currFrame;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
			JDepthTest* depthTest; 
		public:
			JGraphicOccDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JDepthTest* depthTest);
		};
 
		struct JGraphicHzbOccComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public: 
			JFrameResource* currFrame;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
		public:
			JGraphicHzbOccComputeSet(JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM);
		};

		struct JGraphicHdOccExtractSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JCullingManager* cullingM;
		public:
			JGraphicHdOccExtractSet(JCullingManager* cullingM);
		};
 
		struct JGraphicOutlineDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
		public:
			JGraphicOutlineDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM);
		};
		
		struct JGraphicBlurComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device; 
			std::unique_ptr<JBlurDesc> desc;
		public:
			JGraphicBlurComputeSet(JGraphicDevice* device, std::unique_ptr<JBlurDesc>&& desc);
		};

		struct JGraphicDownSampleComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
			std::unique_ptr<JDownSampleDesc> desc; 
		public:
			std::vector<Core::JDataHandle> handle;
		public:
			JGraphicDownSampleComputeSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				std::unique_ptr<JDownSampleDesc>&& desc,
				std::vector<Core::JDataHandle>&& handle);
		};
		 
		struct JGraphicSsaoComputeSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM; 
			JFrameResource* currFrame; 
		public:
			JGraphicSsaoComputeSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, JFrameResource* currFrame);
		};

		struct JGraphicLightCullingTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingManager;
			JFrameResource* currFrame;
		public:
			JGraphicLightCullingTaskSet(JGraphicDevice* device, 
				JGraphicResourceManager* graphicResourceM, 
				JCullingManager* cullingManager,
				JFrameResource* currFrame);
		};

		struct JGraphicLightCullingDebugDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;  
		public:
			JGraphicLightCullingDebugDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM);
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
			std::unique_ptr<JGraphicLightCullingTaskSet> litCulling;
			std::unique_ptr<JGraphicLightCullingDebugDrawSet> litCullingDebug;
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
			std::unique_ptr<JGraphicLightCullingDebugDrawSet> litCullingDebug;
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
	}
}