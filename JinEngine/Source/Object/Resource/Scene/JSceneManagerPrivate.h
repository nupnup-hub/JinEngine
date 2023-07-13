#pragma once
#include"../../Component/JComponentType.h"

namespace JinEngine
{
	class JScene;
	class JSceneManagerPrivate
	{
	public:
		class SceneAccess
		{
			friend class JScene;
		private:
			static bool RegisterScene(const JUserPtr<JScene>& scene)noexcept;
			static bool DeRegisterScene(const JUserPtr<JScene>& scene)noexcept; 
		};
	};
}