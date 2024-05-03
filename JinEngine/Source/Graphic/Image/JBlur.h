#pragma once  
#include"JImageProcessingEnum.h" 
#include"../JGraphicTaskInterface.h" 
#include"../Shader/JShaderDataHandler.h"
#include"../JGraphicSubClassInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDrawHelper;
		class JGraphicResourceInfo;

		class JBlur : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper) = 0;
		};
	}
}