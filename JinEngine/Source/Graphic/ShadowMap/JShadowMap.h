#pragma once  
#include"../DataSet/JGraphicDataSet.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../Thread/JGraphicMultiThreadDrawInterface.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Pointer/JOwnerPtr.h"   

namespace JinEngine
{
	//struct JGraphicShaderData; 
	namespace Graphic
	{
		struct JGraphicInfo;
		struct JDrawHelper;
		class JShadowMap : public JGraphicDeviceUser, public JGraphicMultiThreadDrawInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
		public:
			virtual void BindResource(const JGraphicBindSet* bindSet) = 0;
		public:
			virtual void DrawSceneShadowMap(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShadowMapMultiThread(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper) = 0;
		};
	}
}