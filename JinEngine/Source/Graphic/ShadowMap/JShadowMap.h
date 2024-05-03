#pragma once  
#include"../JGraphicTaskInterface.h"
#include"../JGraphicSubClassInterface.h"
#include"../Thread/JGraphicMultiThreadDrawInterface.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Pointer/JOwnerPtr.h"   

namespace JinEngine
{
	//struct JGraphicShaderData; 
	namespace Graphic
	{
		struct JGraphicInfo;
		class JDrawHelper;
		class JShadowMap : public JGraphicTaskInterface, 
			public JGraphicSubClassInterface,
			public JGraphicMultiThreadDrawInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void BindResource(const JGraphicBindSet* bindSet) = 0;
		public:
			virtual void DrawSceneShadowMap(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShadowMapMultiThread(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper) = 0;
		};
	}
}