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
#include"../../../GameObject/JGameObject.h" 
#include"../../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	JPreviewDirectory::JPreviewDirectory(_In_ Core::JUserPtr<JDirectory> jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(jDir, previewDimension, previewFlag)
	{
		scene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>
			(jDir->GetName() + L"_PreviewScene",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				JResourceManager::Instance().GetEditorResourceDirectory()));

		JSceneManager::Instance().TryOpenScene(scene, true);
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
		jobject.Clear();

		if (textureMaterial != nullptr)
		{
			JObject::BeginDestroy(textureMaterial);
			textureMaterial = nullptr;
		}

		if (scene != nullptr)
		{
			JSceneManager::Instance().TryCloseScene(scene);
			JObject::BeginDestroy(scene); 
			scene = nullptr;
		}
	}
	JScene* JPreviewDirectory::GetScene()noexcept
	{
		return scene;
	}
	bool JPreviewDirectory::MakeJDirectoryPreviewScene()noexcept
	{ 
		JTexture* texture = JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::DIRECTORY);
		if (texture == nullptr)
			return false;

		const std::wstring matName = jobject->GetName() + L"PreviewMaterial";
		JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();

		J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		textureMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(matName, Core::MakeGuid(), flag, dir));
		textureMaterial->SetAlbedoMap(texture);
		textureMaterial->SetAlbedoOnly(true);

		JGameObject* shapeObj = JGFU::CreateShape(*scene->GetRootGameObject(), flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);
		JRenderItem* renderItem = shapeObj->GetRenderItem();

		renderItem->SetMaterial(0, textureMaterial);
		const DirectX::XMFLOAT3 center = renderItem->GetMesh()->GetBoundingSphereCenter();
		const float radius = renderItem->GetMesh()->GetBoundingSphereRadius();

		AdjustCamera(scene, previewCamera, center, radius, true);
		return true;
	}
}
