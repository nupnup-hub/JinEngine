#pragma once
#include"../JGraphicInfo.h"
#include"../JGraphicOption.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Math/JVector.h"
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
			JGraphicResourceManager* gResourceM;
		public:
			JGraphicDeviceInitSet(const JGraphicInfo& info, const JGraphicOption& option, JGraphicResourceManager* gResourceM);
		};
		struct JGraphicGuiInitSet
		{ 
		public:
			JGraphicResourceManager* gResourceM;
			JGraphicDevice* gDevice;
		};
		struct JGraphicDrawReferenceSet : public Core::JValidInterface
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* gResourceM;
			JCullingManager* cullingM;
		public:
			JFrameResource* currFrame;
		public:
			JDepthMapDebug* depthDebug;
			JDepthTest* depthTest;
		public:
			JGraphicDrawReferenceSet(const JGraphicInfo& info, 
				const JGraphicOption& option,
				JGraphicDevice* device,
				JGraphicResourceManager* gResourceM,
				JCullingManager* cullingM,
				JFrameResource* currFrame,
				JDepthMapDebug* depthDebug,
				JDepthTest* depthTest);
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
 
		struct JGraphicDepthMapDebugObjectSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicResourceManager* graphicResourceM;
		public: 
			JGraphicDepthMapDebugObjectSet(JGraphicResourceManager* graphicResourceM);
		};
 
		struct JGraphicDepthMapDebugHandleSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JVector2<uint> size;
			float nearF;
			float farF;
		public:
			JGraphicDepthMapDebugHandleSet(const JVector2<uint> size, const float nearF, const float farF);
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
		public:
			JGraphicShadowMapDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM);
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

		struct JGraphicOutlineObjectSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
		public:
			JGraphicOutlineObjectSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM);
		};

		struct JGraphicOutlineHandleSet : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			JGraphicResourceManager* graphicResourceM;
		public:
			JGraphicOutlineHandleSet(JGraphicResourceManager* graphicResourceM);
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
			std::unique_ptr<JGraphicDepthMapDebugObjectSet> depthMapDebug;
			std::unique_ptr<JGraphicOutlineObjectSet> outline;
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
			std::unique_ptr<JGraphicDepthMapDebugObjectSet> depthMapDebug;
			std::unique_ptr<JGraphicOutlineObjectSet> outline;
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