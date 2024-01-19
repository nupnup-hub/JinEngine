#pragma once  
#include"JImageProcessingEnum.h" 
#include"../JGraphicTaskInterface.h" 
#include"../Shader/JShaderDataHandler.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JDrawHelper; 
		class JGraphicResourceInfo;

		class JImageProcessing : public JGraphicTaskInterface, public JGraphicDeviceShaderHandler
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper) = 0;
			virtual void ApplyMipmapGeneration(JGraphicDownSampleComputeSet* computeSet, const JDrawHelper& helper) = 0;
			virtual void ApplySsao(JGraphicSsaoComputeSet* computeSet, const JDrawHelper& helper) = 0;
		public:
			virtual void TryCreationSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newSsao) = 0;
			virtual void TryDestructionSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* ssao) = 0;
		};
	}
}