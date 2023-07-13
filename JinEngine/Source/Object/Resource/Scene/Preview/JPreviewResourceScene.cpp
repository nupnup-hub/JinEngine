#include"JPreviewResourceScene.h"
#include"../JScene.h"
#include"../JSceneManager.h"
#include"../../JResourceObject.h" 
#include"../../JResourceManager.h" 
#include"../../JResourceObjectType.h" 
#include"../../Material/JMaterial.h" 
#include"../../Material/JMaterialPrivate.h" 
#include"../../Material/JDefaultMaterialSetting.h"
#include"../../Mesh/JMeshGeometry.h"
#include"../../Texture/JTexture.h" 
#include"../../../Component/JComponentCreator.h"
#include"../../../Component/RenderItem/JRenderItem.h"
#include"../../../Component/Camera/JCamera.h"
#include"../../../Component/Camera/JCameraState.h"
#include"../../../GameObject/JGameObject.h"  
#include"../../../GameObject/JGameObjectCreator.h" 
#include"../../../../Graphic/JGraphicDrawList.h"
#include"../../../../Application/JApplicationProject.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Identity/JIdenCreator.h"

namespace JinEngine
{ 
	JPreviewResourceScene::JPreviewResourceScene(_In_ JUserPtr<JResourceObject> resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(resource, previewDimension, previewFlag)
	{
		JUserPtr<JScene> newScene = JICI::Create<JScene>(resource->GetName() + L"_PreviewScene",
			Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			JScene::GetDefaultFormatIndex(),
			_JResourceManager::Instance().GetEditorResourceDirectory(),
			J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW);

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
		JUserPtr<JScene> scene = GetScene(); 
		JUserPtr<JMeshGeometry> mesh = Core::ConvertChildUserPtr<JMeshGeometry>(GetJObject());
		 
		if (mesh == nullptr)
			return false;

		JUserPtr<JGameObject> shapeObj = JICI::Create<JGameObject>(mesh->GetName(), Core::MakeGuid(), OBJECT_FLAG_EDITOR_OBJECT, scene->GetRootGameObject());
		JUserPtr<JRenderItem> renderItem = JCCI::CreateRenderItem(shapeObj, mesh);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeMaterialPreviewScene()noexcept
	{ 
		JUserPtr<JMaterial> material = Core::ConvertChildUserPtr<JMaterial>(GetJObject());
		if (material == nullptr)
			return false;

		_JSceneManager::Instance().RegisterObservationFrame(GetScene(), material->FrameUserInterface());
		JUserPtr<JGameObject> shapeObj = JGCI::CreateShape(GetScene()->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::SPHERE);
		JUserPtr<JRenderItem> renderItem = shapeObj->GetRenderItem();

		renderItem->SetMaterial(0, material);
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeUserTexturePreviewScene()noexcept
	{ 
		JUserPtr<JScene> scene = GetScene();
		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JUserPtr<JResourceObject> rObj = _JResourceManager::Instance().TryGetResourceUser(JTexture::StaticTypeInfo(), resource->GetGuid());
		JUserPtr<JTexture> texture = JUserPtr<JTexture>::ConvertChild(std::move(rObj));
		if (texture == nullptr)
			return false;

		const std::wstring matName = resource->GetName() + L"PreviewMaterial";
		JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();

		const J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JUserPtr<JMaterial> newTextureMat = JICI::Create<JMaterial>(matName, Core::MakeGuid(), flag, JMaterial::GetDefaultFormatIndex(), dir);
	 
		J_DEFAULT_SHAPE shapeType = J_DEFAULT_SHAPE::QUAD;
		if (texture->GetTextureType() == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
		{
			JDefaultMaterialSetting::SetSky(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::SPHERE;
		}
		else
			JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);

		JUserPtr<JGameObject> shapeObj = JGCI::CreateShape(scene->GetRootGameObject(), flag, shapeType);
		JUserPtr<JRenderItem> renderItem = shapeObj->GetRenderItem();
		renderItem->SetMaterial(0, newTextureMat);
		
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		SetTextureMaterial(newTextureMat);
		SetUseQuadShapeTrigger(shapeType == J_DEFAULT_SHAPE::QUAD);
		AdjustCamera(center, radius);
		return true;
	}
	bool JPreviewResourceScene::MakeEditorTexturePreviewScene(const J_DEFAULT_TEXTURE editorTextureType)noexcept
	{ 
		JUserPtr<JScene> scene = GetScene();
		JResourceObject* resource = static_cast<JResourceObject*>(GetJObject().Get());
		JUserPtr<JTexture> texture = _JResourceManager::Instance().GetDefaultTexture(editorTextureType);

		if (texture == nullptr)
			return false;

		const std::wstring matName = resource->GetName() + L"PreviewMaterial";
		JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();

		const J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JUserPtr<JMaterial> newTextureMat = JICI::Create<JMaterial>(matName, Core::MakeGuid(), flag, JMaterial::GetDefaultFormatIndex(), dir);

		J_DEFAULT_SHAPE shapeType = J_DEFAULT_SHAPE::EMPTY;
		if (texture->GetTextureType() == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
		{
			JDefaultMaterialSetting::SetSky(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::SPHERE;
		}
		else
		{
			JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);
			shapeType = J_DEFAULT_SHAPE::QUAD;
		}

		JUserPtr<JGameObject> shapeObj = JGCI::CreateShape(scene->GetRootGameObject(), flag, shapeType);
		JUserPtr<JRenderItem> renderItem = shapeObj->GetRenderItem();
		renderItem->SetMaterial(0, newTextureMat);

		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		SetTextureMaterial(newTextureMat);
		SetUseQuadShapeTrigger(shapeType == J_DEFAULT_SHAPE::QUAD);
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