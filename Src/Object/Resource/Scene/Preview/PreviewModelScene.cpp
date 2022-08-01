#include"PreviewModelScene.h"
#include"../../Scene/JScene.h" 
#include"../../Model/JModel.h"  
#include"../../../Component/Camera/JCamera.h"  
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
		previewCamera.resize(1); 
		JOBJECT_FLAG camFlag = OBJECT_FLAG_EDITOR_OBJECT;
		previewCamera[0] = modelScene->GetMainCamera();
	}
	PreviewModelScene::~PreviewModelScene()
	{

	}
	bool PreviewModelScene::Initialze()noexcept
	{
		if (model != nullptr)
		{
			//modelScene->MakeDefaultObject();
			//modelScene->Activate();   
			AdjustCamera(modelScene, previewCamera[0], model->GetSkeletonCenter(), model->GetSkeletonRadius());	
			if (previewDimension == PREVIEW_DIMENSION::THREE_DIMENTIONAL_RESOURCE)
				Graphic::JGraphicDrawList::AddDrawList(GetScene(), Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS, false);
			else
				Graphic::JGraphicDrawList::AddDrawList(GetScene(), Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED, false);
			return true;
		}
		else
			return false;
	}
	void PreviewModelScene::Clear()noexcept
	{
		PreviewScene::Clear();  
		modelScene = nullptr;
		model = nullptr;	
	}
	JScene* PreviewModelScene::GetScene()noexcept
	{
		return modelScene;
	}
}
