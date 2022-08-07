#include"JGraphicBufInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		int JGraphicBufElementInterface::GetBuffIndex()const noexcept
		{
			return buffIndex;
		}
		void JGraphicBufElementInterface::SetBuffIndex(const int value)noexcept
		{
			buffIndex = value;
		}
		int JGraphicBufUserInterface::GetBuffIndex(JGraphicBufElementInterface& bufEleInterface)const noexcept
		{
			return bufEleInterface.GetBuffIndex();
		}
		void JGraphicBufManagerInterface::SetBuffIndex(JGraphicBufElementInterface& bufEleInterface, int index)noexcept
		{
			bufEleInterface.SetBuffIndex(index);
		}
	}
}