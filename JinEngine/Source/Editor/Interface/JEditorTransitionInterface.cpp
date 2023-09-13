#include"JEditorTransitionInterface.h"

namespace JinEngine
{
	namespace Editor
	{
		Core::JTransition& JEditorTransition::Instance()
		{
			static Core::JTransition trans(Name());
			return trans;
		}
		std::string JEditorTransition::Name()
		{
			return "JEditorTransition";
		}
	}
}
