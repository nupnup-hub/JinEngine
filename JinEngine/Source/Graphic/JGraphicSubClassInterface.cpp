#include"JGraphicSubClassInterface.h" 
#include"JGraphic.h"

namespace JinEngine::Graphic
{   
	const JGraphicInfo& JGraphicSubClassInterface::GetGraphicInfo()const noexcept
	{ 
		return _JGraphic::Instance().GetGraphicInfoRef();
	}
	const JGraphicOption& JGraphicSubClassInterface::GetGraphicOption()const noexcept
	{
		return _JGraphic::Instance().GetGraphicOptionRef();
	}
	bool JGraphicSubClassInterface::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JGraphicSubClassInterface::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		return false;
	}
	void JGraphicSubClassInterface::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{}
	void JGraphicSubClassInterface::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{}
}