#include"JPropertyOptionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		JPropertyOptionInfo::JPropertyOptionInfo(std::unique_ptr<JGuiWidgetInfo>&& widgetInfo)
			:widgetInfo(std::move(widgetInfo))
		{

		}
		JGuiWidgetInfo* JPropertyOptionInfo::GetWidgetInfo()const noexcept
		{
			return widgetInfo.get();
		}
		bool JPropertyOptionInfo::HasWidgetInfo()const noexcept
		{
			return widgetInfo != nullptr;
		}
		bool JPropertyOptionInfo::IsGroupMember()const noexcept
		{
			return widgetInfo != nullptr&& widgetInfo->IsMemberWidget();
		}
	}
}