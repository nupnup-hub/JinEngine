#include"JGraphicEvent.h"
#include"GraphicResource/JGraphicResourceInfo.h"

namespace JinEngine::Graphic
{
	JGraphicOptionChangedEvStruct::JGraphicOptionChangedEvStruct(const JGraphicOption& preOption, const JGraphicOption& newOption)
		:preOption(preOption), newOption(newOption)
	{
		SetValid(true);
	}
	J_GRAPHIC_EVENT_TYPE JGraphicOptionChangedEvStruct::GetEventType()const noexcept
	{
		return J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED;
	}
}