#pragma once 
#include"../DataSet/JGraphicDataSet.h" 
#include"../Device/JGraphicDeviceUser.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		struct JGraphicInfo;
		struct JDrawHelper;
		class JDepthMapDebug : public JGraphicDeviceUser
		{ 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
		public:
			virtual void DrawCamDepthDebug(const JGraphicDepthMapDebugObjectSet* debugSet, const JDrawHelper& helper) = 0;
			virtual void DrawLitDepthDebug(const JGraphicDepthMapDebugObjectSet* debugSet, const JDrawHelper& helper) = 0;
		public:
			virtual void DrawLinearDepthDebug(const JGraphicDepthMapDebugHandleSet* debugSet) = 0;
			virtual void DrawNonLinearDepthDebug(const JGraphicDepthMapDebugHandleSet* debugSet) = 0;
		};
	}
}