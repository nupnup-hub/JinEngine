#include"JPreviewDirectory.h" 
#include"../JScene.h"   
#include"../../JResourceManager.h"
#include"../../Texture/JTexture.h"
#include"../../Material/JMaterial.h"
#include"../../Material/JDefaultMaterialSetting.h"
#include"../../Mesh/JMeshGeometry.h" 
#include"../../../Directory/JDirectory.h"
#include"../../../Component/JComponentCreator.h"
#include"../../../Component/RenderItem/JRenderItem.h" 
#include"../../../GameObject/JGameObject.h" 
#include"../../../GameObject/JGameObjectCreator.h" 
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Core/Identity/JIdenCreator.h"

#include "../../../../Application/JApplicationProject.h"
namespace JinEngine
{
	JPreviewDirectory::JPreviewDirectory(_In_ Core::JUserPtr<JDirectory> jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(jDir, previewDimension, previewFlag)
	{ 
		Core::JUserPtr<JScene> newScene = JICI::CreateRetUser<JScene>(jDir->GetName() + L"_PreviewScene",
			Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			JScene::GetDefaultFormatIndex(),
			_JResourceManager::Instance().GetEditorResourceDirectory(),
			J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW);

		SetScene(newScene); 
	}
	JPreviewDirectory::~JPreviewDirectory(){}
	bool JPreviewDirectory::Initialze()noexcept
	{
		if (MakeJDirectoryPreviewScene())
			return true;
		else
		{
			Clear();
			return false;
		}
	}
	bool JPreviewDirectory::MakeJDirectoryPreviewScene()noexcept
	{ 
		Core::JUserPtr<JTexture> texture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::DIRECTORY);
		if (texture == nullptr)
			return false;

		const std::wstring matName = GetJObject()->GetName() + L"PreviewMaterial";
		JDirectory* dir = _JResourceManager::Instance().GetEditorResourceDirectory();

		J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JMaterial* newTextureMat = JICI::Create<JMaterial>(matName, Core::MakeGuid(), flag, JMaterial::GetDefaultFormatIndex(), dir);
		JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);
		 
		JGameObject* shapeObj = JGCI::CreateShape(GetScene()->GetRootGameObject(), flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);
		JRenderItem* renderItem = shapeObj->GetRenderItem();

		renderItem->SetMaterial(0, Core::GetUserPtr(newTextureMat));
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		SetTextureMaterial(Core::GetUserPtr(newTextureMat));
		SetUseQuadShapeTrigger(true);
		AdjustCamera(center, radius);
		return true;
	}
}
