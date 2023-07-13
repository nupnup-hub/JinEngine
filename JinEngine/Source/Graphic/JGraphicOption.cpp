#include"JGraphicOption.h"
namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicOption::IsHDOccActivated()const noexcept
		{
			return isOcclusionQueryActivated && isHDOcclusionAcitvated;
		}
		bool JGraphicOption::IsHZBOccActivated()const noexcept
		{
			return isOcclusionQueryActivated && isHZBOcclusionActivated;
		}
	}
}