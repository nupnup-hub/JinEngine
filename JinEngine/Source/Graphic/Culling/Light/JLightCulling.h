#pragma once  
#include"../../Shader/JShaderDataHandler.h"
#include"../../JGraphicTaskInterface.h" 
#include"../../JGraphicSubClassInterface.h"
#include"../../Thread/JGraphicMultiThreadDrawInterface.h"
#include<memory> 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JDrawHelper;
		class JLightCulling : public JGraphicTaskInterface,
			public JGraphicSubClassInterface,
			public JGraphicDeviceShaderHandler
		{
		public: 
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;  
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void NotifyNewClusterOption(JGraphicDevice* device) = 0;
			virtual void NotifyLocalLightCapacityChanged(JGraphicDevice* device, JGraphicResourceManager* gM, const size_t capacity) = 0;
		public:
			virtual void BindDrawResource(const JGraphicBindSet* bindSet) = 0; 
			virtual void BindDebugResource(const JGraphicBindSet* bindSet) = 0;
		public:
			virtual void ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper) = 0;
			virtual void ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper) = 0;
		public: 
			virtual void StreamOutDebugInfo(const std::wstring& path) = 0;
		};
	}
}