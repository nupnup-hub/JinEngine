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
			virtual void DrawCamDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet, const JDrawHelper& helper) = 0;
			virtual void DrawLitDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet, const JDrawHelper& helper) = 0;
		public:
			virtual void DrawLinearDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet) = 0;
			virtual void DrawNonLinearDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet) = 0;
		};
	}
}