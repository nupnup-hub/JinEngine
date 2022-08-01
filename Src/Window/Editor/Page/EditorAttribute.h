#pragma once
#include<string>

namespace JinEngine
{
	class EditorAttribute
	{
	public:
		//don't push space in name
		const std::string name;
		const size_t guid;
		const float initPosXRate;
		const float initPosYRate;
		const float initWidthRate;
		const float initHeightRate;
		bool isOpen = false;
		bool isFront = false;	
		bool isActivated = false;
		bool isFocus = false;
	public:
		EditorAttribute(const std::string& name,
			const size_t guid, 
			float initPosXRate,
			float initPosYRate, 
			float initWidthRate,
			float initHeightRate,
			bool isOpen,
			bool isFront);
		~EditorAttribute() = default;
		EditorAttribute(const EditorAttribute&) = default;
	};
}