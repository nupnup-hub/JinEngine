#include"PreviewModelScene.h"
#include"../../Scene/JScene.h" 
#include"../../Model/JModel.h"  
#include"../../../Component/Camera/JCamera.h"  
#include"../../../GameObject/JGameObject.h"
#include"../../../GameObject/JGameObjectFactoryUtility.h"
#include"../../../../Utility/JMathHelper.h" 
#include"../../../../Graphic/JGraphicDrawList.h" 
#include<Windows.h>

namespace JinEngine
{
	PreviewModelScene::PreviewModelScene(const std::string& previewSceneName, _In_ JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag)
		:PreviewScene(previewSceneName, resource, previewDimension, previewFlag)
	{
		model = static_cast<JModel*>(resource);
		modelScene = model->GetModelScene(); 

		if (GetResourceReferenceCount(*modelScene) == 0)
			previewCamera = modelScene->GetMainCamera();
		else
		{
			JGameObject* camObj = JGFU::CreateCamera(*modelScene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, false, "EditorCam");
			previewCamera = camObj->GetComponent<JCamera>();
			previewCamera->StateInterface()->SetCameraState(J_CAMERA_STATE::RENDER);
		}	 
		OnSceneReference();
	}
	PreviewModelScene::~PreviewModelScene()
	{

	}
	bool PreviewModelScene::Initialze()noexcept
	{
		if (model != nullptr)
		{ 
			AdjustCamera(modelScene, previewCamera, model->GetSkeletonCenter(), model->GetSkeletonRadius());	 
			return true;
		}
		else
			return false;
	}
	void PreviewModelScene::Clear()noexcept
	{ 
		if (previewCamera->GetGuid() != modelScene->GetMainCamera()->GetGuid())
			JGameObject::EraseGameObject(previewCamera->GetOwner());
		OffSceneReference();
		model = nullptr;	
	}
	JScene* PreviewModelScene::GetScene()noexcept
	{
		return modelScene;
	}
}
