#pragma once
#include"../../Core/Transition/JTransition.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorTransition
		{
		public:
			static Core::JTransition& Instance();
			static std::string Name();
		};
	}
} 
