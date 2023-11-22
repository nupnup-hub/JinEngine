#pragma once
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Editor
	{
		class JEditorAniContInterface
		{
		public:
			virtual void SetAnimationController(const JUserPtr<JAnimationController>& newAniCont) = 0;
		};
	}
}