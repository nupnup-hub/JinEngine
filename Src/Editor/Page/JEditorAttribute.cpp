#include"JEditorAttribute.h" 

namespace JinEngine
{
	namespace Editor
	{
		JEditorAttribute::JEditorAttribute(float initPosXRate,
			float initPosYRate,
			float initWidthRate,
			float initHeightRate,
			bool isOpen,
			bool isLastAct)
			:initPosXRate(initPosXRate),
			initPosYRate(initPosYRate),
			initWidthRate(initWidthRate),
			initHeightRate(initHeightRate),
			isOpen(isOpen),
			isLastAct(isLastAct)
		{}
		bool JEditorAttribute::IsFullScreenInit()noexcept
		{
			return initPosXRate == 0 && initPosYRate == 0 && initWidthRate == 1 && initHeightRate == 1;
		}
	}
}