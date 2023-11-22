#pragma once  
#include"JImageProcessingEnum.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../Device/JGraphicDeviceUser.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JDrawHelper;
		class JImageProcessing : public JGraphicDeviceUser
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0; 
		public:
			virtual void ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper) = 0;
			virtual void ApplyMipmapGeneration(JGraphicDownSampleTaskSet* taskSet, const JDrawHelper& helper) = 0;
			virtual void ApplySsao(JGraphicSsaoTaskSet* taskSet, const JDrawHelper& helper) = 0;
		};
	}
}