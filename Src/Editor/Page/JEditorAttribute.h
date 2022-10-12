#pragma once
#include<string>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute
		{
		public:
			//don't push space in name 
			const float initPosXRate;
			const float initPosYRate;
			const float initWidthRate;
			const float initHeightRate;
			bool isOpen = false; 
			bool isFocus = false;
			bool isActivated = false;
			bool isLastAct = false;
		public:
			JEditorAttribute(float initPosXRate,
				float initPosYRate,
				float initWidthRate,
				float initHeightRate,
				bool isOpen,
				bool isLastAct);
			~JEditorAttribute() = default;
			JEditorAttribute(const JEditorAttribute&) = default;
		public:
			bool IsFullScreenInit()noexcept;
		};
	}
}