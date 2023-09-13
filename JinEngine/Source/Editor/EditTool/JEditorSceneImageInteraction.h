#pragma once 
#include"../../Core/Pointer/JOwnerPtr.h" 
#include"../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"

namespace JinEngine
{
	class JGameObject;
	class JCamera;
	class JScene;
	namespace Editor
	{
		class JEditorSceneImageInteraction
		{
		public:
			static JUserPtr<JGameObject> Intersect(JUserPtr<JScene> scene,
				JUserPtr<JCamera> cam,
				const J_ACCELERATOR_LAYER layer,
				const JVector2<float>& sceneImageMinPoint) noexcept;
			//use obb
			static std::vector<JUserPtr<JGameObject>> Contain(JUserPtr<JScene> scene,
				JUserPtr<JCamera> cam,
				const J_ACCELERATOR_LAYER layer,
				const JVector2<float>& sceneImageMinPoint,
				const JVector2<float>& minMousePos,
				const JVector2<float>& maxMousePos) noexcept;
		};
	}
}