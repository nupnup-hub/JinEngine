#pragma once  
#include"JImageProcessingEnum.h" 
#include"../JGraphicTaskInterface.h" 
#include"../JGraphicSubClassInterface.h"
#include"../Shader/JShaderDataHandler.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDrawHelper;
		class JGraphicResourceInfo;

		class JPostProcessExposure : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void ExtractExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) = 0;
			virtual void UpdateExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) = 0;
		};
	}
}