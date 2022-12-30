#include"JEditorAttribute.h" 

namespace JinEngine
{
	namespace Editor
	{
		JEditorAttribute::JEditorAttribute(float initPosXRate,
			float initPosYRate,
			float initWidthRate,
			float initHeightRate)
			:initPosXRate(initPosXRate),
			initPosYRate(initPosYRate),
			initWidthRate(initWidthRate),
			initHeightRate(initHeightRate)
		{}
		bool JEditorAttribute::IsFullScreenInit()noexcept
		{
			return initPosXRate == 0 && initPosYRate == 0 && initWidthRate == 1 && initHeightRate == 1;
		}
	}
}