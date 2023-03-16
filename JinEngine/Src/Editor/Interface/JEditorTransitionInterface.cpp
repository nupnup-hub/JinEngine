#include"JEditorTransitionInterface.h"

namespace JinEngine
{
	namespace Editor
	{
		Core::JTransition& JEditorTransition::Instance()
		{
			static Core::JTransition trans;
			return trans;
		}
	}
}
