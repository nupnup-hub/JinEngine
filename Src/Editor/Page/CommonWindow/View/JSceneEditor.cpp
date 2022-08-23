#include"JSceneEditor.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h"
#include"../../../Utility/JEditorUtility.h" 
#include"../../../Window/JWindows.h"
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Core/GameTimer/JGameTimer.h"  
#include"../../../../../Lib/imgui/imgui.h"    

namespace JinEngine
{
	namespace Editor
	{
		JSceneEditor::JSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
		}
		JSceneEditor::~JSceneEditor()
		{

		}
	}
}


/*
void JSceneEditor::UpdateAfterRendering(Graphic* graphic, JCamera* sceneCamera, _In_ JGameTimer* JGameTimer, JEditorUtility* editorUtility)
{
		if (isSceneSizeUpdated)
		{
			sceneCamera.SetWindowSize(attribute->width, attribute->height);
			isSceneSizeUpdated = false;
		}
	}
*/