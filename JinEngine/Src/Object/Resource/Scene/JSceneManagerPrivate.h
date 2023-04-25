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
			static bool RegisterScene(JScene* scene, bool isPreviewScene)noexcept;
			static bool DeRegisterScene(JScene* scene)noexcept;
		private:
			static void UpdateScene(JScene* scene, const J_COMPONENT_TYPE compType);
		};
	};
}