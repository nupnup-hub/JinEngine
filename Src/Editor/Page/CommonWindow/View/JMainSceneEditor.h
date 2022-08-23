#pragma once
#include"JSceneEditor.h" 
#include<DirectXMath.h>

namespace JinEngine
{
	class JCamera;
	class JScene;
	class JGameObject;

	namespace Editor
	{
		class JMainSceneEditor : public JSceneEditor
		{
		private:
			JCamera* editorCamera;
			JGameObject* camObj;
			JGameObject* frustumObj;
			JScene* mainScene;
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 rotation;
		public:
			JMainSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JMainSceneEditor();
			JMainSceneEditor(const JMainSceneEditor& rhs) = delete;
			JMainSceneEditor& operator=(const JMainSceneEditor& rhs) = delete;

			void Initialize()noexcept;
			bool Activate() final;
			bool DeActivate() final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
			void UpdateWindow()override;
		private:
			void DrawGameObjectWidget();
		};
	}
}