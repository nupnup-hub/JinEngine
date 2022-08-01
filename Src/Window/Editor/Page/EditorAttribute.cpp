#include"EditorAttribute.h" 

namespace JinEngine
{
	EditorAttribute::EditorAttribute(const std::string& name,
		const size_t guid,
		float initPosXRate,
		float initPosYRate,
		float initWidthRate, 
		float initHeightRate,
		bool isOpen,
		bool isFront)
		:name(name), 
		guid(guid),
		initPosXRate(initPosXRate),
		initPosYRate(initPosYRate),
		initWidthRate(initWidthRate),
		initHeightRate(initHeightRate),
		isFront(isFront),
		isOpen(isOpen)
	{}
}