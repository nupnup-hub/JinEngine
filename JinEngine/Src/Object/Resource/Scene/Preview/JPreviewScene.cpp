#include"JPreviewScene.h"  
#include"../JScene.h" 
#include"../JSceneManager.h"
#include"../../Material/JMaterial.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/Camera/JCamera.h" 
#include"../../../Component/Transform/JTransform.h" 
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JPreviewScene::JPreviewScene(JUserPtr<JObject>  jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:guid(Core::MakeGuid()), jobject(jobject), previewDimension(previewDimension), previewFlag(previewFlag)
	{}
	JPreviewScene::~JPreviewScene() {}
	size_t JPreviewScene::GetGuid()const noexcept
	{
		return guid;
	}
	JUserPtr<JObject> JPreviewScene::GetJObject()noexcept
	{ 
		return jobject;
	}
	JUserPtr<JCamera> JPreviewScene::GetPreviewCamera()noexcept
	{
		return camera;
	}
	J_PREVIEW_DIMENSION JPreviewScene::GetPreviewDimension()const noexcept
	{
		return previewDimension;
	}
	void JPreviewScene::SetUseQuadShapeTrigger(const bool value)noexcept
	{
		useQuadShape = value;
	}
	void JPreviewScene::SetScene(JUserPtr<JScene> newScene)noexcept
	{
		scene = newScene;
		camera = scene->FindFirstSelectedCamera(true);
		camera->SetAllowFrustumCulling(false);
		camera->SetAllowHzbOcclusionCulling(false);
	}
	bool JPreviewScene::UseQuadShape()const noexcept
	{
		return useQuadShape;
	}
	void JPreviewScene::Clear()noexcept
	{
		camera = nullptr;
		jobject.Clear();

		if (textureMaterial != nullptr)
		{
			JObject::BeginDestroy(textureMaterial.Get());
			textureMaterial = nullptr;
		}

		if (scene != nullptr)
		{ 
			JObject::BeginDestroy(scene.Get());
			scene = nullptr;
		}
	}
	void JPreviewScene::AdjustCamera(_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{ 
		if (previewDimension == J_PREVIEW_DIMENSION::TWO_DIMENTIONAL)
		{
			if (useQuadShape)
				Adjust2DTextureCamera(objCenter, objRadius, additionalPos);
			else
				Adjust2DOtherCamera(objCenter, objRadius, additionalPos);
		}
		else
		{
			if (((int)previewFlag & (int)J_PREVIEW_FLAG::NON_FIXED) > 0)
				Adjust3DNonFixedCamera(objCenter, objRadius, additionalPos);
			else
				Adjust3DFixedCamera(objCenter, objRadius, additionalPos);
		}
	}
	void JPreviewScene::Adjust2DTextureCamera(_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos{ objCenter.x + additionalPos.x, objCenter.y + additionalPos.y, objCenter.z - 1 + additionalPos.z };
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetOrthoCamera(true);
		camera->SetViewSize((int)(objRadius), (int)(objRadius));
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust2DOtherCamera(_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos{ objCenter.x + additionalPos.x, objCenter.y + additionalPos.y, objCenter.z + (objRadius * 1.25f) + additionalPos.z };
		camera->GetTransform()->SetPosition(camNewPos);
		//camera->SetOrthoCamera();
		camera->SetViewSize((int)(objRadius * 1.325f), (int)(objRadius * 1.325f));
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust3DFixedCamera(_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos = objCenter;
		camNewPos.x += objRadius + additionalPos.x;
		camNewPos.y += objRadius + additionalPos.y;
		camNewPos.z += objRadius + additionalPos.z;
		camera->GetTransform()->SetPosition(camNewPos); 
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust3DNonFixedCamera(_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos = objCenter;
		camNewPos.x += objRadius + additionalPos.x;
		camNewPos.y += objRadius + additionalPos.y;
		camNewPos.z += objRadius + additionalPos.z;
		camera->GetTransform()->SetPosition(camNewPos); 
		camera->GetTransform()->LookAt(objCenter);
	}
}