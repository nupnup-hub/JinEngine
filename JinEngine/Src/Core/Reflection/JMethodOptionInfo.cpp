#include"JMethodOptionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		JMethodOptionInfo::JMethodOptionInfo(std::unique_ptr<JGuiWidgetInfo>&& widgetInfo)
			:widgetInfo(std::move(widgetInfo))
		{

		}
		JGuiWidgetInfo* JMethodOptionInfo::GetWidgetInfo()const noexcept
		{
			return widgetInfo.get();
		}
		bool JMethodOptionInfo::HasWidgetInfo()const noexcept
		{
			return widgetInfo != nullptr;
		}
		bool JMethodOptionInfo::IsGroupMember()const noexcept
		{
			return widgetInfo != nullptr && widgetInfo->IsMemberWidget();
		}
	}
}