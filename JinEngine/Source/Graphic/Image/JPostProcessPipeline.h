#pragma once  
#include"JImageProcessingEnum.h"  
#include"../DataSet/JGraphicDataSet.h"
#include"../JGraphicSubClassInterface.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JDrawHelper;
		class JGraphicResourceInfo;

		class JPostProcessPipeline : public JGraphicSubClassInterface
		{
		public:
			void ApplyPostProcess(JPostProcessComputeSet* computeSet, const JDrawHelper& helper, const bool isUpdatedThisFrame);
		};
	}
}