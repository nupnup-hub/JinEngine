#include"PreviewScene.h"  
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/Camera/JCamera.h" 
#include"../../../Component/Transform/JTransform.h" 
#include"../../../../Graphic/JGraphicDrawList.h" 
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	PreviewScene::PreviewScene(const std::string& previewSceneName, JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag)
		:name(previewSceneName), resource(resource), previewDimension(previewDimension), previewFlag(previewFlag)
	{
		guid = JCommonUtility::CalculateGuid(previewSceneName);
	}
	PreviewScene::~PreviewScene() {}
	void PreviewScene::Clear()noexcept
	{
		Graphic::JGraphicDrawList::PopDrawList(GetScene());
	}
	JResourceObject* PreviewScene::GetResouceObject()noexcept
	{
		return resource;
	}
	JCamera* PreviewScene::GetPreviewCamera(const uint index)noexcept
	{
		return previewCamera[index];
	}
	PREVIEW_DIMENSION PreviewScene::GetPreviewDimension()const noexcept
	{
		return previewDimension;
	}
	void PreviewScene::AdjustCamera(_In_ JScene* scene,
		_Inout_ JCamera* camera,
		_In_ const DirectX::XMFLOAT3& objCenter,
		const float objRadius,
		bool isQuad,
		const DirectX::XMFLOAT3 additionalPos)noexcept
	{
		if (previewDimension == PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE)
		{
			if (isQuad)
				Adjust2DTextureCamera(scene, camera, objCenter, objRadius, additionalPos);
			else
				Adjust2DOtherCamera(scene, camera, objCenter, objRadius, additionalPos);
		}
		else
		{
			if (((int)previewFlag & (int)PREVIEW_FLAG::NON_FIXED) > 0)
				Adjust3DNonFixedCamera(scene, camera, objCenter, objRadius, additionalPos);
			else
				Adjust3DFixedCamera(scene, camera, objCenter, objRadius, additionalPos);
		}
	}
	void PreviewScene::Adjust2DTextureCamera(_In_ JScene* scene,
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
	void PreviewScene::Adjust2DOtherCamera(_In_ JScene* scene,
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
	void PreviewScene::Adjust3DFixedCamera(_In_ JScene* scene,
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
	void PreviewScene::Adjust3DNonFixedCamera(_In_ JScene* scene,
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