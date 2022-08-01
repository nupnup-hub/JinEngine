#pragma once
#include"../EditorWindow.h"  
#include"../../../../Utility/Vector.h"
#include<memory>

namespace JinEngine
{     
	class EditorCameraControl;
	class SceneEditor : public EditorWindow
	{ 
	protected: 
		std::unique_ptr<EditorCameraControl> editorCamCtrl;
	public:
		SceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~SceneEditor(); 
		SceneEditor(const SceneEditor& rhs) = delete;
		SceneEditor& operator=(const SceneEditor& rhs) = delete;
	};
}