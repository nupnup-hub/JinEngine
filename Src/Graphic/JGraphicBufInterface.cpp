#include"JGraphicBufInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		int JGraphicBufElementInterface::GetGraphicBuffIndex()const noexcept
		{
			return buffIndex;
		}
		void JGraphicBufElementInterface::SetGraphicBuffIndex(const int value)noexcept
		{
			buffIndex = value;
		}
		int JGraphicBufUserInterface::GetGraphicBuffIndex(JGraphicBufElementInterface& bufEleInterface)const noexcept
		{
			return bufEleInterface.GetGraphicBuffIndex();
		}
		void JGraphicBufManagerInterface::SetGraphicBuffIndex(JGraphicBufElementInterface& bufEleInterface, int index)noexcept
		{
			bufEleInterface.SetGraphicBuffIndex(index);
		}
	}
}