#include"JPreviewDirectory.h" 
#include"../JScene.h"  
#include"../JSceneManager.h"
#include"../../Texture/JTexture.h"
#include"../../Material/JMaterial.h" 
#include"../../Mesh/JMeshGeometry.h"
#include"../../JResourceObjectFactory.h"
#include"../../../Directory/JDirectory.h"
#include"../../../Component/JComponentFactoryUtility.h"
#include"../../../Component/RenderItem/JRenderItem.h" 
#include"../../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	JPreviewDirectory::JPreviewDirectory(_In_ JDirectory* jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(jDir, previewDimension, previewFlag)
	{
		scene = JRFI<JScene>::Create(jDir->GetName() + "_PreviewScene",
			Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			*JResourceManager::Instance().GetEditorResourceDirectory(),
			JResourceObject::GetInvalidFormatIndex());

		JSceneManager::Instance().TryOpenScene(scene);
		previewCamera = scene->GetMainCamera();
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
	void JPreviewDirectory::Clear()noexcept
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
	JScene* JPreviewDirectory::GetScene()noexcept
	{
		return scene;
	}
	bool JPreviewDirectory::MakeJDirectoryPreviewScene()noexcept
	{ 
		JTexture* texture = JResourceManager::Instance().GetEditorTexture(J_EDITOR_TEXTURE::DIRECTORY);
		if (texture == nullptr)
			return false;

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
		textureMaterial = JRFI<JMaterial>::Create(*JResourceManager::Instance().GetEditorResourceDirectory());
		textureMaterial->SetName(jobject->GetName() + "PreviewMaterial");
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
