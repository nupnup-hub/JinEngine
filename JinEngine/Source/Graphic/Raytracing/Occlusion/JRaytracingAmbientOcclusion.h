#pragma once  
#include"../../JGraphicTaskInterface.h"
#include"../../JGraphicSubClassInterface.h" 
#include"../../../Core/JCoreEssential.h"     

namespace JinEngine
{
	namespace Graphic
	{
		class JDrawHelper;
		class JRaytracingAmbientOcclusion : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final; 
		public:
			virtual void StreamOutDebugInfo() = 0;
		public:
			virtual void ComputeAmbientOcclusion(const JGraphicRtAoComputeSet* computeSet, const JDrawHelper& helper) = 0;
		};
	}
}