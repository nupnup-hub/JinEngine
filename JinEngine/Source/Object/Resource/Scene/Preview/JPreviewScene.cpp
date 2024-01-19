#include"JPreviewScene.h"  
#include"../JScene.h" 
#include"../JSceneManager.h"
#include"../../Material/JMaterial.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/Camera/JCamera.h" 
#include"../../../Component/Transform/JTransform.h" 
#include"../../../Component/RenderItem/JRenderItem.h" 
#include"../../../Component/Light/JLight.h"
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Window/JWindow.h"
 
namespace JinEngine
{
	JPreviewScene::JPreviewScene(JUserPtr<JObject>  jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:guid(Core::MakeGuid()), jobject(jobject), previewDimension(previewDimension), previewFlag(previewFlag)
	{}
	JPreviewScene::~JPreviewScene() 
	{ 
		Clear(); 
	}
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
		camera->SetRenderTargetRate(JVector2F(128, 128) / JWindow::GetDisplaySize());
		camera->GetOwner()->SetName(jobject->GetName() + L"_PreviewCamera");
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
	void JPreviewScene::AdjustScene(const AdjustSceneSettingData& data)noexcept
	{ 
		if (previewDimension == J_PREVIEW_DIMENSION::TWO_DIMENTIONAL)
		{
			if (useQuadShape)
				Adjust2DTextureScene(data);
			else
				Adjust2DScene(data);
		}
		else
		{
			if (((int)previewFlag & (int)J_PREVIEW_FLAG::NON_FIXED) > 0)
				Adjust3DNonFixedScene(data);
			else
				Adjust3DFixedScene(data);
		}
	}
	void JPreviewScene::Adjust2DTextureScene(const AdjustSceneSettingData& data)noexcept
	{ 
		auto bbox = data.targetRenderItem->GetBoundingBox();
		auto pad = data.useFixedPad ? JVector2F(0.05f, 0.005f) : data.pad;

		JVector3F camNewPos{ bbox.Center.x, bbox.Center.y, bbox.Center.z - 1};
		camera->GetTransform()->SetPosition(camNewPos);
		camera->SetOrthoCamera(true);
		camera->SetOrthoViewSize((bbox.Extents.x - pad.x) * 2, (bbox.Extents.y - pad.y) * 2);
		camera->GetTransform()->LookAt(bbox.Center);
	}
	void JPreviewScene::Adjust2DScene(const AdjustSceneSettingData& data)noexcept
	{ 
		auto bSphere = data.targetRenderItem->GetBoundingSphere();
		auto bbox = data.targetRenderItem->GetBoundingBox();

		float radius = JVector2F(bbox.Extents.x, bbox.Extents.y).Length();
		camera->SetAspect(1.0f);
		 
		constexpr float zOffset = 0.4f;
		const float sphereRadius = bSphere.Radius;
		const float z = camera->GetNear() + bSphere.Radius;
		const float r = std::tan(camera->GetFovY() * 0.5f) * z;
		const float xyFactor = (r * 2.0) / bSphere.Radius;
		const float zFactor = (r * 2.0f) / bSphere.Radius;

		const JVector3F s = JVector3F(xyFactor - data.pad.x, xyFactor - data.pad.y, zFactor);
		data.targetTransform->SetScale(s);

		JVector3F zeroExtentAdjustRot = JVector3F::Zero();
		bSphere = data.targetRenderItem->GetBoundingSphere();
		bbox = data.targetRenderItem->GetBoundingBox();
		if (bbox.Extents.x <= 0.1f)
			zeroExtentAdjustRot.y += 75.0f;
		if (bbox.Extents.y <= 0.1f)
			zeroExtentAdjustRot.x -= 15.0f;
		if (bbox.Extents.z <= 0.1f)
			zeroExtentAdjustRot.y += 15.0f;

		JVector3F additionalPos = data.additionalPosRate * bSphere.Radius;
		JVector3F camNewPos{bSphere.Center.x, bSphere.Center.y, z + zOffset * bSphere.Radius};
		camera->GetTransform()->SetPosition(camNewPos + additionalPos);
		camera->GetTransform()->SetRotation(camera->GetTransform()->GetRotation() + zeroExtentAdjustRot);
		camera->GetTransform()->LookAt(bSphere.Center);

		auto dLight = scene->GetFirstDirectionalLight();
		dLight->SetPower(2.0f);
		dLight->GetOwner()->GetTransform()->SetRotation(JVector3F(75, 0, 0));
	}
	void JPreviewScene::Adjust3DFixedScene(const AdjustSceneSettingData& data)noexcept
	{
		auto bSphere = data.targetRenderItem->GetBoundingSphere();
		JVector3F camNewPos = bSphere.Center + data.additionalPos + CreateVec3(bSphere.Radius);
		camera->GetTransform()->SetPosition(camNewPos); 
		camera->GetTransform()->LookAt(bSphere.Center);
		
		auto dLight = scene->GetFirstDirectionalLight();
		dLight->SetPower(2.0f);
		dLight->GetOwner()->GetTransform()->SetRotation(JVector3F(25, 0, 0));
	}
	void JPreviewScene::Adjust3DNonFixedScene(const AdjustSceneSettingData& data)noexcept
	{
		auto bSphere = data.targetRenderItem->GetBoundingSphere();
		JVector3F camNewPos = bSphere.Center + data.additionalPos + CreateVec3(bSphere.Radius);
		camera->GetTransform()->SetPosition(camNewPos); 
		camera->GetTransform()->LookAt(bSphere.Center);
		 
		auto dLight = scene->GetFirstDirectionalLight();
		dLight->SetPower(2.0f);
		dLight->GetOwner()->GetTransform()->SetRotation(JVector3F(25, 0, 0));
	}
}