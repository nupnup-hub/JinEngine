#pragma once 
#include"../DataSet/JGraphicDataSet.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../Shader/JShaderDataHandler.h"
#include"../Thread/JGraphicMultiThreadDrawInterface.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Pointer/JOwnerPtr.h"  
namespace JinEngine
{
	class JGameObject;
	namespace Graphic
	{
		struct JDrawHelper; 
		struct JGraphicInfo; 

		class JSceneDraw : public JGraphicDeviceUser,
			public JGraphicShaderDataHandler,
			public JGraphicMultiThreadDrawInterface
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0; 
		public:
			virtual void BindResource(const JGraphicBindSet* bindSet) = 0;
		public:
			//Contain Draw Debug UI
			virtual void DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet,const JDrawHelper& helper) = 0;
		};
	}
}