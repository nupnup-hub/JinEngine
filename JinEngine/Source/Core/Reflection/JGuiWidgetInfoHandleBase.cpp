#include"JGuiWidgetInfoHandleBase.h" 
#include"JGuiWidgetInfo.h"

namespace JinEngine
{
	namespace Core
	{ 
		JGuiWidgetInfoHandleBase::~JGuiWidgetInfoHandleBase(){}

		uint JGuiWidgetInfoHandleBase::GetWidgetInfoCount()const noexcept
		{
			return (uint)widgetInfo.size();
		}
		JUserPtr<JGuiWidgetInfo> JGuiWidgetInfoHandleBase::GetWidgetInfo(const uint index)const noexcept
		{
			return widgetInfo[index];
		}
	}
}