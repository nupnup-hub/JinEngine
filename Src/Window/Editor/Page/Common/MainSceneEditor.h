#pragma once
#include"SceneEditor.h" 
#include<DirectXMath.h>

namespace JinEngine
{
	class JCamera;
	class JScene;
	class JGameObject;

	class MainSceneEditor : public SceneEditor
	{
	private:
		JCamera* editorCamera;
		JGameObject* camObj;
		JGameObject* frustumObj; 
		JScene* mainScene;
		DirectX::XMFLOAT3 position; 
		DirectX::XMFLOAT3 rotation;
	public:
		MainSceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~MainSceneEditor();
		MainSceneEditor(const MainSceneEditor& rhs) = delete;
		MainSceneEditor& operator=(const MainSceneEditor& rhs) = delete;
		  
		void Initialize(EditorUtility* editorUtility)noexcept;
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final; 
		void StoreEditorWindow(std::wofstream& stream)final;
		void LoadEditorWindow(std::wifstream& stream)final;
		void UpdateWindow(EditorUtility* editorUtility)override;
	private:
		void DrawGameObjectWidget(EditorUtility* editorUtility); 
	};
}