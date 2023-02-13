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
			bool isOpen = false; 
			bool isFocus = false;
			bool isActivated = false;
			bool isLastAct = false;
		public:
			JEditorAttribute() = default;
			~JEditorAttribute() = default;
			JEditorAttribute(const JEditorAttribute&) = default; 
		};
	}
}