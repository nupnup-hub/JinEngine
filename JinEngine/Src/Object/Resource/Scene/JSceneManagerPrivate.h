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
			static bool RegisterScene(const JUserPtr<JScene>& scene, bool isPreviewScene)noexcept;
			static bool DeRegisterScene(const JUserPtr<JScene>& scene)noexcept;
		private:
			static void UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE compType);
		};
	};
}