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
			const std::string& name;
			const size_t guid;
			const float initPosXRate;
			const float initPosYRate;
			const float initWidthRate;
			const float initHeightRate;
			bool isOpen = false;
			bool isFront = false;
			bool isFocus = false;
			bool isActivated = false;
		public:
			JEditorAttribute(const std::string& name,
				const size_t guid,
				float initPosXRate,
				float initPosYRate,
				float initWidthRate,
				float initHeightRate,
				bool isOpen,
				bool isFront);
			~JEditorAttribute() = default;
			JEditorAttribute(const JEditorAttribute&) = default;
		};
	}
}