#include"JPreviewScene.h"  
#include"../Jscene.h" 
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/Camera/JCamera.h" 
#include"../../../Component/Transform/JTransform.h" 
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JPreviewScene::JPreviewScene(JObject* jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:guid(Core::MakeGuid()), jobject(jobject), previewDimension(previewDimension), previewFlag(previewFlag)
	{}
	JPreviewScene::~JPreviewScene() {}
	size_t JPreviewScene::GetGuid()const noexcept
	{
		return guid;
	}
	JObject* JPreviewScene::GetJObject()noexcept
	{ 
		return jobject;
	}
	JCamera* JPreviewScene::GetPreviewCamera()noexcept
	{
		return previewCamera;
	}
	J_PREVIEW_DIMENSION JPreviewScene::GetPreviewDimension()const noexcept
	{
		return previewDimension;
	}
	void JPreviewScene::AdjustCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		bool isQuad,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		if (previewDimension == J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE)
		{
			if (isQuad)
				Adjust2DTextureCamera(scene, camera, objCenter, objRadius, additionalPos);
			else
				Adjust2DOtherCamera(scene, camera, objCenter, objRadius, additionalPos);
		}
		else
		{
			if (((int)previewFlag & (int)J_PREVIEW_FLAG::NON_FIXED) > 0)
				Adjust3DNonFixedCamera(scene, camera, objCenter, objRadius, additionalPos);
			else
				Adjust3DFixedCamera(scene, camera, objCenter, objRadius, additionalPos);
		}
	}
	void JPreviewScene::Adjust2DTextureCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos{ objCenter.x + additionalPos.x, objCenter.y + additionalPos.y, objCenter.z - 1 + additionalPos.z };
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetOrthoCamera();
		camera->SetViewSize((int)(objRadius), (int)(objRadius));
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust2DOtherCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos{ objCenter.x + additionalPos.x, objCenter.y + additionalPos.y, objCenter.z + (objRadius * 1.25f) + additionalPos.z };
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetOrthoCamera();
		camera->SetViewSize((int)(objRadius * 1.325f), (int)(objRadius * 1.325f));
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust3DFixedCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos = objCenter;
		camNewPos.x += objRadius + additionalPos.x;
		camNewPos.y += objRadius + additionalPos.y;
		camNewPos.z += objRadius + additionalPos.z;
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetPerspectiveCamera();
		camera->GetTransform()->LookAt(objCenter);
	}
	void JPreviewScene::Adjust3DNonFixedCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		DirectX::XMFLOAT3 camNewPos = objCenter;
		camNewPos.x += objRadius + additionalPos.x;
		camNewPos.y += objRadius + additionalPos.y;
		camNewPos.z += objRadius + additionalPos.z;
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetPerspectiveCamera();
		camera->GetTransform()->LookAt(objCenter);
	}
}