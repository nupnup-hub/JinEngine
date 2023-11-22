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
		class JDepthMapDebug;
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
			JDepthMapDebug* depthDebug;
			JDepthTest* depthTest;
		public:
			JImageProcessing* image;
		public:
			JGraphicDrawReferenceSet(const JGraphicInfo& info, 
				const JGraphicOption& option,
				JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JFrameResource* currFrame,
				JDepthMapDebug* depthDebug,
				JDepthTest* depthTest,
				JImageProcessing* image);
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
   
		struct JGraphicDepthMapDebugTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* gDevice = nullptr;
			JGraphicResourceManager* graphicResourceM = nullptr;
		public:
			JVector2<uint> size = JVector2<uint>::Zero();
			float nearF = 0;
			float farF = 0;
			bool isPerspective = true;
		public:
			JGraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice, JGraphicResourceManager* graphicResourceM);
			JGraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice, 
				JGraphicResourceManager* graphicResourceM,
				const JVector2<uint> size, 
				const float nearF, 
				const float farF, 
				const bool isPerspective);
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

		struct JGraphicOccDebugDrawSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device; 
			JGraphicResourceManager* graphicResourceM; 
			JDepthMapDebug* depthDebug;
		public:
			JGraphicOccDebugDrawSet(JGraphicDevice* device, 
				JGraphicResourceManager* graphicResourceM, 
				JDepthMapDebug* depthDebug);
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
		
		struct JGraphicBlurTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device; 
			std::unique_ptr<JBlurDesc> desc;
		public:
			JGraphicBlurTaskSet(JGraphicDevice* device, std::unique_ptr<JBlurDesc>&& desc);
		};

		struct JGraphicDownSampleTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
			std::unique_ptr<JDownSampleDesc> desc; 
		public:
			std::vector<Core::JDataHandle> handle;
		public:
			JGraphicDownSampleTaskSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				std::unique_ptr<JDownSampleDesc>&& desc,
				std::vector<Core::JDataHandle>&& handle);
		};
		 
		struct JGraphicSsaoTaskSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM; 
			JFrameResource* currFrame; 
		public:
			JGraphicSsaoTaskSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, JFrameResource* currFrame);
		};

		//draw scene single thread
		struct JGraphicDrawSceneSTSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicSceneDrawSet> sceneDraw;
			std::unique_ptr<JGraphicShadowMapDrawSet> shadowMapDraw;
			std::unique_ptr<JGraphicOccDrawSet> occDraw;
			std::unique_ptr<JGraphicOccDebugDrawSet> occDebug;
			std::unique_ptr<JGraphicHzbOccComputeSet> hzbCompute;
			std::unique_ptr<JGraphicHdOccExtractSet> hdExtract;
			std::unique_ptr<JGraphicDepthMapDebugTaskSet> depthMapDebug;
			std::unique_ptr<JGraphicOutlineDrawSet> outline; 
		};

		struct JGraphicBeginFrameSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
		};

		struct JGraphicMidFrameSet
		{
		public:
			std::unique_ptr<JGraphicBindSet> bind;
			std::unique_ptr<JGraphicSceneDrawSet> sceneDraw;
			std::unique_ptr<JGraphicOccDebugDrawSet> occDebug;
			std::unique_ptr<JGraphicHzbOccComputeSet> hzbCompute;
			std::unique_ptr<JGraphicHdOccExtractSet> hdExtract;
			std::unique_ptr<JGraphicDepthMapDebugTaskSet> depthMapDebug;
			std::unique_ptr<JGraphicOutlineDrawSet> outline;
			std::unique_ptr<JGraphicSsaoTaskSet> ssao;
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