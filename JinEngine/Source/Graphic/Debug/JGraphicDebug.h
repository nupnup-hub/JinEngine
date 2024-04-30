#pragma once 
#include"../JGraphicTaskInterface.h"
#include"../JGraphicSubClassInterface.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		struct JGraphicInfo;
		class JDrawHelper;
		class JGraphicDebug : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void ComputeCamDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper) = 0;
			virtual void ComputeLitDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper) = 0;
		};
	}
}