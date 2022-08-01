#include"SceneEditor.h" 
#include"EditorCameraControl.h"
#include"../EditorAttribute.h"
#include"../../Utility/EditorUtility.h" 
#include"../../../JWindows.h"
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Core/GameTimer/JGameTimer.h"  
#include"../../../../../Lib/imgui/imgui.h"    

namespace JinEngine
{
	SceneEditor::SceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{
		editorCamCtrl = std::make_unique<EditorCameraControl>();
	}
	SceneEditor::~SceneEditor()
	{

	}
}


/*
void SceneEditor::UpdateAfterRendering(Graphic* graphic, JCamera* sceneCamera, _In_ JGameTimer* JGameTimer, EditorUtility* editorUtility)
{
		if (isSceneSizeUpdated)
		{
			sceneCamera.SetWindowSize(attribute->width, attribute->height);
			isSceneSizeUpdated = false;
		}
	}
*/