#pragma once
#include"../../Core/Reflection/JTypeInfo.h" 
namespace JinEngine
{
	namespace Editor
	{
		struct JDragDropData
		{
		public:
			std::vector<Core::JTypeInstanceSearchHint> hintVec;
		};
	}
}