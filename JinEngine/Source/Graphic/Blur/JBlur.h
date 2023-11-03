#pragma once  
#include"JBlurEnum.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../Device/JGraphicDeviceUser.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JDrawHelper;
		class JBlur : public JGraphicDeviceUser
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0; 
		public:
			virtual void ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper) = 0;
		};
	}
}