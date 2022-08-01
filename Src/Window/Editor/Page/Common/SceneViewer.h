#pragma once
#include"../EditorWindow.h"
#include"../../../../Utility/JDelegate.h"    
#include"../../../../Utility/Vector.h"    

namespace JinEngine
{ 
	class JCamera;
	class EditorCameraControl;
	class SceneViewer : public EditorWindow
	{
	private:
		JCamera* selectedCamera;
		std::unique_ptr<EditorCameraControl> editorCamCtrl;

		//수정필요
		//포트폴리오용코드
		bool onAnimation = false;
	public:
		SceneViewer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~SceneViewer();
		SceneViewer(const SceneViewer& rhs) = delete;
		SceneViewer& operator=(const SceneViewer& rhs) = delete;
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
		void StoreEditorWindow(std::wofstream& stream)final;
		void LoadEditorWindow(std::wifstream& stream)final;
		void UpdateWindow(EditorUtility* editorUtility)override;
	};
}
