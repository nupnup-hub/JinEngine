#include"JPreviewResourceScene.h"
#include"../JScene.h"  
#include"../JSceneManager.h"
#include"../../JResourceObject.h" 
#include"../../JResourceManager.h" 
#include"../../JResourceType.h" 
#include"../../Material/JMaterial.h" 
#include"../../Mesh/JMeshGeometry.h"
#include"../../Texture/JTexture.h"
#include"../../JResourceObjectFactory.h"
#include"../../../Component/JComponentFactoryUtility.h"
#include"../../../Component/RenderItem/JRenderItem.h"
#include"../../../Component/Camera/JCamera.h"
#include"../../../Component/Camera/JCameraState.h"
#include"../../../GameObject/JGameObject.h" 
#include"../../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../../../Graphic/JGraphicDrawList.h"
#include"../../../../Application/JApplicationVariable.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	JPreviewResourceScene::JPreviewResourceScene(_In_ JResourceObject* resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(resource, previewDimension, previewFlag)
	{
		scene = JRFI<JScene>::Create(resource->GetName() + "_PreviewScene",
			Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			*JResourceManager::Instance().GetEditorResourceDirectory(),
			JResourceObject::GetInvalidFormatIndex());

		JSceneManager::Instance().TryOpenScene(scene);
		previewCamera = scene->GetMainCamera(); 
	}
	JPreviewResourceScene::~JPreviewResourceScene() {}
	bool JPreviewResourceScene::Initialze()noexcept
	{
		//scene->MakeDefaultObject();
		JResourceObject* resource = static_cast<JResourceObject*>(jobject);
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
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::SCRIPT);
			break;
		case J_RESOURCE_TYPE::SHADER:
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::SHADER);
			break;
		case J_RESOURCE_TYPE::SKELETON:
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::SKELETON);
			break;
		case J_RESOURCE_TYPE::ANIMATION_CLIP:
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::ANIMATION_CLIP);
			break;
		case J_RESOURCE_TYPE::SCENE:
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::SCENE);
			break;
		case J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			res = MakeEditorTexturePreviewScene(J_EDITOR_TEXTURE::ANIMATION_CONTROLLER);
			break;
		default:
			res = false;
			break;
		}
		if (!res)
		{
			MessageBox(0, L"실패", resource->GetWName().c_str(), 0);
			Clear();
		}
		return res;
	}
	void JPreviewResourceScene::Clear()noexcept
	{
		previewCamera = nullptr;
		jobject = nullptr;

		if (textureMaterial != nullptr)
		{
			textureMaterial->BeginDestroy();
			textureMaterial = nullptr;
		}

		if (scene != nullptr)
		{
			JSceneManager::Instance().TryCloseScene(scene);
			scene->BeginDestroy();
			scene = nullptr; 
		}
	}
	JScene* JPreviewResourceScene::GetScene()noexcept
	{
		return scene;
	}
	bool JPreviewResourceScene::MakeMeshPreviewScene()noexcept
	{
		JResourceObject* resource = static_cast<JResourceObject*>(jobject);
		JMeshGeometry* mesh = JResourceManager::Instance().GetResource<JMeshGeometry>(resource->GetGuid());

		if (mesh == nullptr)
			return false;

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
		JRenderItem* renderItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT, 
			*shapeObj,
			mesh,
			JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD));

		renderItem->GetMaterial()->SetNonCulling(true);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(scene, previewCamera, center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeMaterialPreviewScene()noexcept
	{
		JResourceObject* resource = static_cast<JResourceObject*>(jobject);
		JMaterial* material = JResourceManager::Instance().GetResource<JMaterial>(resource->GetGuid());

		if (material == nullptr)
			return false;

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
		JRenderItem* renderItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			*shapeObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE),
			material);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(scene, previewCamera, center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeUserTexturePreviewScene()noexcept
	{
		JResourceObject* resource = static_cast<JResourceObject*>(jobject);
		JTexture* texture = JResourceManager::Instance().GetResource<JTexture>(resource->GetGuid());
		if (texture == nullptr)
			return false;

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
		textureMaterial = JRFI<JMaterial>::Create(*JResourceManager::Instance().GetEditorResourceDirectory()); 
		textureMaterial->SetName(resource->GetName() + "PreviewMaterial");
		textureMaterial->SetAlbedoMap(texture);
		textureMaterial->SetAlbedoOnly(true);

		JRenderItem* renderItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			*shapeObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE),
			textureMaterial);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(scene, previewCamera, center, radius, true);
		return true;
	}
	bool JPreviewResourceScene::MakeEditorTexturePreviewScene(const J_EDITOR_TEXTURE editorTextureType)noexcept
	{
		JResourceObject* resource = static_cast<JResourceObject*>(jobject);
		JTexture* texture = JResourceManager::Instance().GetEditorTexture(editorTextureType);

		if (texture == nullptr)
			return false;

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
		textureMaterial = JRFI<JMaterial>::Create(*JResourceManager::Instance().GetEditorResourceDirectory());
		textureMaterial->SetName(resource->GetName() + "PreviewMaterial");
		textureMaterial->SetAlbedoMap(texture);
		textureMaterial->SetAlbedoOnly(true);


		JRenderItem* renderItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			*shapeObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE),
			textureMaterial);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(scene, previewCamera, center, radius, true);
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