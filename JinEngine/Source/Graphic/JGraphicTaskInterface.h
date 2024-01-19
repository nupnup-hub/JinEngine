#pragma once  
#include"JGraphicEnum.h"
#include"DataSet/JGraphicDataSet.h"
#include"Device/JGraphicDeviceUser.h"
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicTaskInterface : public JGraphicDeviceUser
		{
		public:
			virtual bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		};
	}
}
