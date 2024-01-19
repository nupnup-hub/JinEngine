#pragma once 
#include"../JGraphicTaskInterface.h"
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

		class JSceneDraw : public JGraphicTaskInterface,
			public JGraphicShaderDataHandler,
			public JGraphicMultiThreadDrawInterface
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet) = 0;
			virtual void Clear() = 0; 
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet) = 0;
		public:
			//Contain Draw Debug UI
			virtual void DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet,const JDrawHelper& helper) = 0;
			virtual void DrawSceneDebugUIMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShade(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShadeMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
		};
	}
}