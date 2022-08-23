#include"JPreviewModelScene.h"
#include"../../Scene/JScene.h" 
#include"../../Scene/JSceneManager.h"
#include"../../Model/JModel.h"  
#include"../../../Component/Camera/JCamera.h"  
#include"../../../GameObject/JGameObject.h"
#include"../../../GameObject/JGameObjectFactoryUtility.h"
#include"../../../../Utility/JMathHelper.h" 
#include"../../../../Graphic/JGraphicDrawList.h" 
#include<Windows.h>

namespace JinEngine
{
	JPreviewModelScene::JPreviewModelScene(_In_ JResourceObject* resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(resource, previewDimension, previewFlag)
	{
		model = static_cast<JModel*>(resource);
		modelScene = model->ModelSceneInterface()->GetModelScene();

		if (!JSceneManager::Instance().TryOpenScene(modelScene))
			previewCamera = modelScene->GetMainCamera();
		else
		{ 
			JGameObject* camObj = JGFU::CreateCamera(*modelScene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, false, "EditorCam");
			previewCamera = camObj->GetComponent<JCamera>();
			previewCamera->StateInterface()->SetCameraState(J_CAMERA_STATE::RENDER);
		} 
	}
	JPreviewModelScene::~JPreviewModelScene() {}
	bool JPreviewModelScene::Initialze()noexcept
	{
		if (model != nullptr)
		{ 
			AdjustCamera(modelScene, previewCamera, model->GetSkeletonCenter(), model->GetSkeletonRadius());	 
			return true;
		}
		else
			return false;
	}
	void JPreviewModelScene::Clear()noexcept
	{ 
		if (previewCamera->GetGuid() != modelScene->GetMainCamera()->GetGuid())
			previewCamera->BeginDestroy();
		JSceneManager::Instance().TryCloseScene(modelScene);
		previewCamera = nullptr;
		jobject = nullptr;
		modelScene = nullptr;
		model = nullptr;	
	}
	JScene* JPreviewModelScene::GetScene()noexcept
	{
		return modelScene;
	}
}
