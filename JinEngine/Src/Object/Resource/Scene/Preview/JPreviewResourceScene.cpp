#include"JPreviewResourceScene.h"
#include"../JScene.h"   
#include"../../JResourceObject.h" 
#include"../../JResourceManager.h" 
#include"../../JResourceType.h" 
#include"../../Material/JMaterial.h" 
#include"../../Material/JDefaultMaterialSetting.h"
#include"../../Mesh/JMeshGeometry.h"
#include"../../Texture/JTexture.h"
#include"../../JResourceObjectFactory.h"
#include"../../../Component/JComponentFactoryUtility.h"
#include"../../../Component/RenderItem/JRenderItem.h"
#include"../../../Component/Camera/JCamera.h"
#include"../../../Component/Camera/JCameraState.h"
#include"../../../GameObject/JGameObject.h" 
#include"../../../GameObject/JGameObjectFactory.h" 
#include"../../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../../../Graphic/JGraphicDrawList.h"
#include"../../../../Application/JApplicationVariable.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	JPreviewResourceScene::JPreviewResourceScene(_In_ Core::JUserPtr<JResourceObject> resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(resource, previewDimension, previewFlag)
	{
		JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>
			(resource->GetName() + L"_PreviewScene",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				JResourceManager::Instance().GetEditorResourceDirectory(),
				J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW));
		SetScene(newScene);
	}
	JPreviewResourceScene::~JPreviewResourceScene() {}
	bool JPreviewResourceScene::Initialze()noexcept
	{
		//scene->MakeDefaultObject();
		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		const J_RESOURCE_TYPE resourceType = resource->GetResourceType();
		bool res = false;
		switch (resourceType)
		{
		case J_RESOURCE_TYPE::MESH:
			res = MakeMeshPreviewScene();
			break;
		case J_RESOURCE_TYPE::MATERIAL:
			res = MakeMaterialPreviewScene();
			break;
		case J_RESOURCE_TYPE::TEXTURE:
			res = MakeUserTexturePreviewScene();
			break;
		case J_RESOURCE_TYPE::SCRIPT:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::SCRIPT);
			break;
		case J_RESOURCE_TYPE::SHADER:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::SHADER);
			break;
		case J_RESOURCE_TYPE::SKELETON:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::SKELETON);
			break;
		case J_RESOURCE_TYPE::ANIMATION_CLIP:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::ANIMATION_CLIP);
			break;
		case J_RESOURCE_TYPE::SCENE:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::SCENE);
			break;
		case J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			res = MakeEditorTexturePreviewScene(J_DEFAULT_TEXTURE::ANIMATION_CONTROLLER);
			break;
		default:
			res = false;
			break;
		}
		if (!res)
		{
			MessageBox(0, L"실패", resource->GetName().c_str(), 0);
			Clear();
		}
		return res;
	}
	bool JPreviewResourceScene::MakeMeshPreviewScene()noexcept
	{
		TryOpenScene();
		JScene* scene = GetScene();
		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JMeshGeometry* mesh = JResourceManager::Instance().GetResource<JMeshGeometry>(resource->GetGuid());

		if (mesh == nullptr)
			return false;

		JGameObject* shapeObj = JGFI::Create(mesh->GetName(), Core::MakeGuid(), OBJECT_FLAG_EDITOR_OBJECT, *scene->GetRootGameObject());
		JRenderItem* renderItem = JCFU::CreateRenderItem(Core::MakeGuid(), *shapeObj, mesh);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeMaterialPreviewScene()noexcept
	{
		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JMaterial* material = JResourceManager::Instance().GetResource<JMaterial>(resource->GetGuid());
		if (material == nullptr)
			return false;

		TryOpenScene(Core::GetUserPtr(material));
		JScene* scene = GetScene();

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
		JRenderItem* renderItem = shapeObj->GetRenderItem();

		renderItem->SetMaterial(0, material);
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeUserTexturePreviewScene()noexcept
	{
		TryOpenScene();
		JScene* scene = GetScene();

		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JTexture* texture = JResourceManager::Instance().GetResource<JTexture>(resource->GetGuid());
		if (texture == nullptr)
			return false;

		const std::wstring matName = resource->GetName() + L"PreviewMaterial";
		JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();

		const J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JMaterial* newTextureMat = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(matName, Core::MakeGuid(), flag, dir));
	 
		J_DEFAULT_SHAPE shapeType = J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD;
		if (texture->GetTextureType() == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
		{
			JDefaultMaterialSetting::SetSky(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE;
		}
		else
			JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), flag, shapeType);
		JRenderItem* renderItem = shapeObj->GetRenderItem();
		renderItem->SetMaterial(0, newTextureMat);
		
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		SetTextureMaterial(newTextureMat);
		SetUseQuadShapeTrigger(shapeType == J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);
		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeEditorTexturePreviewScene(const J_DEFAULT_TEXTURE editorTextureType)noexcept
	{
		TryOpenScene();
		JScene* scene = GetScene();

		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JTexture* texture = JResourceManager::Instance().GetDefaultTexture(editorTextureType);

		if (texture == nullptr)
			return false;

		const std::wstring matName = resource->GetName() + L"PreviewMaterial";
		JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();

		const J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JMaterial* newTextureMat = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(matName, Core::MakeGuid(), flag, dir));

		J_DEFAULT_SHAPE shapeType = J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY;
		if (texture->GetTextureType() == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
		{
			JDefaultMaterialSetting::SetSky(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE;
		}
		else
		{
			JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD;
		}

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), flag, shapeType);
		JRenderItem* renderItem = shapeObj->GetRenderItem();
		renderItem->SetMaterial(0, newTextureMat);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		SetTextureMaterial(newTextureMat);
		SetUseQuadShapeTrigger(shapeType == J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);
		AdjustCamera(center, radius);
		return true;
	}
}

/*
수정필요
	void JPreviewResourceScene::OnResourceEvent(const size_t senderGuid, const RESOURCE_EVENT eventType, JResourceObject* resource)
	{
		if (eventType == RESOURCE_EVENT::UPDATE)
		{
			if (JPreviewScene::resource->GetGuid() == resource->GetGuid())
				sceneFrameDirty->SetAllRenderItemDirty();
		}
	}
*/