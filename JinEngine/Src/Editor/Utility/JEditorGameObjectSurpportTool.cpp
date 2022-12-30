#include"JEditorGameObjectSurpportTool.h"
#include"../Page/JEditorPageShareData.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Object/Component/Camera/JCamera.h"  
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Component/JComponentFactory.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectFactory.h"
#include"../../Object/GameObject/JGameObjectFactoryUtility.h"   
#include"../../Object/Resource/Material/JDefaultMaterialType.h" 
#include"../../Object/Resource/Scene/JScene.h" 
#include"../../Object/Resource/JResourceManager.h" 
#include"../../Object/Resource/JResourceObjectFactory.h" 
#include"../../Core/Geometry/JRay.h"


//Debug
#include"../../Debug/JDebugTextOut.h"
using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{
		void JEditorTransformTool::Arrow::CreateMaterial(const JVector4<float> matColor)
		{
			JEditorTransformTool::Arrow::matColor = matColor;

			JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();
			JMaterial* material = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"ArrowMaterial",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				dir));

			material->SetAlbedoColor(matColor.ConvertXMF());
			material->SetDebugMaterial(true);
			//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);

			JEditorTransformTool::Arrow::material = Core::GetUserPtr(material);
		}
		void JEditorTransformTool::Arrow::Initialze(JGameObject* debugRoot,
			const J_DEFAULT_SHAPE shape,
			const JVector3<float> initScale,
			const JVector3<float> initRotation,
			const JVector3<float> initMovePos)
		{
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* arrow = JGFU::CreateShape(*debugRoot, flag, shape);
			JTransform* transform = arrow->GetTransform();

			transform->SetScale(initScale.ConvertXMF());
			transform->SetRotation(initRotation.ConvertXMF());
			transform->SetPosition(initMovePos.ConvertXMF());

			JRenderItem* rItem = arrow->GetRenderItem();
			const uint subMeshCount = rItem->GetSubmeshCount();
			for (uint j = 0; j < subMeshCount; ++j)
				rItem->SetMaterial(j, material.Get());
			rItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_LAYER);

			JEditorTransformTool::Arrow::arrow = Core::GetUserPtr(arrow);
		}

		void JEditorTransformTool::Arrow::Clear()
		{
			if (arrow.IsValid())
				JObject::BeginDestroy(arrow.Release());
			if (material.IsValid())
				JObject::BeginDestroy(material.Release());
		}
		bool JEditorTransformTool::Arrow::IsValid()const noexcept
		{
			return arrow.IsValid() && material.IsValid();
		}
		void JEditorTransformTool::Arrow::SetHoveredColor()noexcept
		{
			material->SetAlbedoColor((matColor + JVector4<float>(0.15f, 0.15f, 0.15f, 0.15f)).ConvertXMF());
		}
		void JEditorTransformTool::Arrow::OffHoveredColor()noexcept
		{
			material->SetAlbedoColor(matColor.ConvertXMF());
		}
		JEditorTransformTool::JEditorTransformTool(const J_DEFAULT_SHAPE shape, const float sizeRate)
			:shape(shape), sizeRate(sizeRate)
		{
			const XMFLOAT3 extents = JResourceManager::Instance().GetDefaultMeshGeometry(shape)->GetBoundingBox().Extents;
			shapeLength = JMathHelper::Vector3Length(extents) * 2;

			JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();
			JMaterial* material = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"ArrowRootMaterial",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				dir));

			material->SetAlbedoColor(XMFLOAT4(0.7f, 0.7f, 0.7f, 0.7f));
			material->SetDebugMaterial(true);
			//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS);

			arrowCenterMaterial = Core::GetUserPtr(material);

			JVector4<float> color[Constants::arrowCount]
			{
				JVector4<float>{0.8f, 0, 0, 0.75f},
				JVector4<float>{0, 0.8f, 0, 0.75f},
				JVector4<float>{0, 0, 0.8f, 0.75f},
			};

			for (uint i = 0; i < Constants::arrowCount; ++i)
				arrow[i].CreateMaterial(color[i]);
		}
		JEditorTransformTool::~JEditorTransformTool()
		{
			for (uint i = 0; i < 3; ++i)
				arrow[i].Clear();
			if (transformArrowRoot.IsValid())
				JObject::BeginDestroy(transformArrowRoot.Get());
			if (arrowCenter.IsValid())
				JObject::BeginDestroy(arrowCenter.Get());
			if (arrowCenterMaterial.IsValid())
				JObject::BeginDestroy(arrowCenterMaterial.Get());
		}
		void JEditorTransformTool::Activate()
		{
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* transformArrowRoot = JGFI::Create(L"Transform Arrow Root", Core::MakeGuid(), flag, *debugRoot);

			//x y z
			JVector3<float> scale[3]
			{
				JVector3<float>{0.85f, 0.85f, 0.85f},
				JVector3<float>{0.85f, 0.85f, 0.85f},
				JVector3<float>{0.85f, 0.85f, 0.85f}
			};
			JVector3<float> rot[3]
			{
				JVector3<float>{0, 0, -90},
				JVector3<float>{0, 0, 0},
				JVector3<float>{90, 0, 0},
			};
			JVector3<float> pos[3]
			{
				JVector3<float>{0.11f, 0, 0},
				JVector3<float>{0, 0.11f, 0},
				JVector3<float>{0, 0, 0.11f},
			};

			for (uint i = 0; i < 3; ++i)
				arrow[i].Initialze(transformArrowRoot, shape, scale[i], rot[i], pos[i]);

			JGameObject* arrowCenter = JGFU::CreateShape(*transformArrowRoot, flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
			JRenderItem* arrowCenterRItem = arrowCenter->GetRenderItem();
			arrowCenterRItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_LAYER);
			arrowCenterRItem->SetMaterial(0, arrowCenterMaterial.Get());

			arrowCenter->GetTransform()->SetScale(XMFLOAT3(0.35f, 0.35f, 0.35f));
			JEditorTransformTool::transformArrowRoot = Core::GetUserPtr(transformArrowRoot);
			JEditorTransformTool::arrowCenter = Core::GetUserPtr(arrowCenter);

			isActivated = true;
			isDraggingObject = false;
			draggingIndex = -1;
		}
		void JEditorTransformTool::DeActivate()
		{
			if (transformArrowRoot.IsValid())
				JGameObject::BeginDestroy(transformArrowRoot.Get());

			isActivated = false;
			isDraggingObject = false;
			draggingIndex = -1;
		}
		void JEditorTransformTool::Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam)
		{
			if (!selected.IsValid() || !cam.IsValid() || selected->GetObjectType() != J_OBJECT_TYPE::GAME_OBJECT)
			{
				if (isActivated)
					DeActivate();
				return;
			}

			JGameObject* gameObject = static_cast<JGameObject*>(selected.Get());
			JRenderItem* rItem = gameObject->GetRenderItem();

			if (rItem != nullptr && (rItem->GetRenderLayer() == J_RENDER_LAYER::DEBUG_LAYER ||
				rItem->GetRenderLayer() == J_RENDER_LAYER::SKY))
			{
				if (isActivated)
					DeActivate();
				return;
			}

			if (!isActivated)
				Activate();

			if (!IsValid())
				return;

			UpdateSelectedTransform(gameObject);
			UpdateArrowPosition(gameObject, cam);
			UpdateArrowDragging(gameObject, cam);
		}
		void JEditorTransformTool::UpdateSelectedTransform(JGameObject* selected)
		{
			if (!ImGui::IsMouseDragging(0))
				return;

			ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0);
		}
		void JEditorTransformTool::UpdateArrowPosition(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{
			JRenderItem* rItem = selected->GetRenderItem();
			if (rItem)
			{
				transformArrowRoot->GetTransform()->SetPosition(rItem->GetBoundingBox().Center);
			}
			else
			{
				const XMMATRIX worldV = selected->GetTransform()->GetWorld();
				XMFLOAT4X4 worldF;
				XMStoreFloat4x4(&worldF, worldV);
				transformArrowRoot->GetTransform()->SetPosition(XMFLOAT3(worldF._41, worldF._42, worldF._43));
			}
		}
		void JEditorTransformTool::UpdateArrowDragging(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{
			if (!ImGui::IsMouseDragging(0))
			{
				isDraggingObject = false;
				draggingIndex = -1; 
			}
			
			const ImVec2 wndSize = ImGui::GetWindowSize();
			const float aspect = wndSize.x / wndSize.y;
			const XMMATRIX projV = XMMatrixPerspectiveFovLH(cam->GetFovY(), aspect, cam->GetNear(), cam->GetFar());
			const XMMATRIX invView = XMMatrixInverse(nullptr, cam->GetView());

			//const float vx = ((2.0f * ImGui::GetMousePos().x) / wndSize.x - 1.0f) / (1 / (aspect * fov));
			//const float vy = ((-2.0f * ImGui::GetMousePos().y) / wndSize.y + 1.0f) / (1 / fov);
			XMFLOAT4X4 projF;
			XMStoreFloat4x4(&projF, projV);
			//const float vx = ((2.0f * ImGui::GetMousePos().x) / wndSize.x - 1.0f) / projF(0, 0);
			//const float vy = ((2.0f * (ImGui::GetMainViewport()->Size.y - ImGui::GetMousePos().y)) / wndSize.y - 1.0f) / projF(1, 1);
			 
			const float vx = ((2.0f * ImGui::GetMousePos().x) / wndSize.x - 1.0f) / projF(0, 0);
			const float vy = ((-2.0f * ImGui::GetMousePos().y) / wndSize.y + 1.0f) / projF(1, 1);
			const XMVECTOR rayOri = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invView);
			const XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(vx, vy, 1.0f, 0.0f), invView));

			//if (ImGui::IsMouseDragging(0))
			//{
				//Core::JDebugTextOut::MessageVector4(rayOri, L"RayOri");
				//Core::JDebugTextOut::MessageVector4(rayDir, L"RayDir");
			//} 
			
			JScene* scene = transformArrowRoot->GetOwnerScene();
			JGameObject* hitObj = scene->Intersect(Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT, Core::JRay{ rayOri, rayDir });
			if (hitObj != nullptr)
			{
				for (uint i = 0; i < Constants::arrowCount; ++i)
				{
					if (arrow[i].arrow->GetGuid() == hitObj->GetGuid())
						arrow[i].SetHoveredColor();
					else
						arrow[i].OffHoveredColor();
				}
			}
			else
			{
				for (uint i = 0; i < Constants::arrowCount; ++i)
					arrow[i].OffHoveredColor();
			}

		}
		void JEditorTransformTool::UpdateSelectedPosition(JEditorTransformTool* tool, const JVector2<float> mouseDelta)
		{

		}
		void JEditorTransformTool::UpdateSelectedRotation(JEditorTransformTool* tool, const JVector2<float> mouseDelta)
		{

		}
		bool JEditorTransformTool::IsValid()const noexcept
		{
			return transformArrowRoot.IsValid() && arrow[0].IsValid()
				&& arrow[1].IsValid()
				&& arrow[2].IsValid();
		}
		void JEditorTransformTool::SetDebugRoot(Core::JUserPtr<JGameObject> debugRoot)
		{
			JEditorTransformTool::debugRoot = debugRoot;
			if (isActivated)
			{
				DeActivate();
				Activate();
			}
		}
	}
}