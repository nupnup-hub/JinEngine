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
#include"../../Core/Geometry/JBBox.h"

//Debug
#include"../../Debug/JDebugTextOut.h"
using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{		
		bool JEditorGameObjectSurpportTool::IsEditable(JGameObject* obj)const noexcept
		{
			return !obj->HasFlag(OBJECT_FLAG_UNEDITABLE);
		}
		JGameObject* JEditorGameObjectSurpportTool::SceneIntersect(Core::JUserPtr<JScene> scene, 
			Core::JUserPtr<JCamera> cam,
			Core::J_SPACE_SPATIAL_LAYER layer) noexcept
		{
			if (!scene.IsValid() || !cam.IsValid())
				return nullptr;
			 
			const ImVec2 windowPos = ImGui::GetWindowPos();
			const ImVec2 windowSize = ImGui::GetWindowSize();
			if (!JImGuiImpl::IsMouseInRect(windowPos, windowSize))
				return nullptr;

			const ImVec2 localMousePos = ImGui::GetMousePos() - JImGuiImpl::GetWorldCursorPos();
			const ImVec2 camViewSize = ImVec2(cam->GetViewWidth(), cam->GetViewHeight());
			const float widthRate = (float)camViewSize.x / windowSize.x;
			const float heightRate = (float)camViewSize.y / windowSize.y;
			const ImVec2 finalMousePos = ImVec2(localMousePos.x * widthRate, localMousePos.y * heightRate);

			const XMFLOAT4X4 proj = cam->GetProj4x4f();
			const XMMATRIX invView = XMMatrixInverse(nullptr, cam->GetView());

			const float vx = ((2.0f * finalMousePos.x) / cam->GetViewWidth() - 1.0f) / proj(0, 0);
			const float vy = ((-2.0f * finalMousePos.y) / cam->GetViewHeight() + 1.0f) / proj(1, 1);
			const XMVECTOR rayOri = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invView);
			const XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(vx, vy, 1.0f, 0.0f), invView));

			return scene->Intersect(layer, Core::JRay{ rayOri, rayDir });
		}
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
			material->SetNonCulling(true);
			//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);

			JEditorTransformTool::Arrow::material = Core::GetUserPtr(material);
		}
		void JEditorTransformTool::Arrow::Initialze(JGameObject* debugRoot,
			const J_DEFAULT_SHAPE shape,
			const JVector3<float> initRotation,
			const JVector3<float> initMovePos)
		{ 
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* newArrow = JGFU::CreateShape(*debugRoot, flag, shape);
			JTransform* transform = newArrow->GetTransform();
			 
			transform->SetRotation(initRotation.ConvertXMF());
			transform->SetPosition(initMovePos.ConvertXMF());

			JRenderItem* rItem = newArrow->GetRenderItem();
			const uint subMeshCount = rItem->GetSubmeshCount();
			for (uint j = 0; j < subMeshCount; ++j)
				rItem->SetMaterial(j, material.Get());
			rItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_UI);

			arrow = Core::GetUserPtr(newArrow);
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
		JEditorTransformTool::JEditorTransformTool(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType, 
			const J_DEFAULT_SHAPE shape,
			const float sizeRate)
			:toolType(toolType), 
			shape(shape), 
			sizeRate(sizeRate), 
			transformUpdatePtr(GetUpdateTransformPtr(toolType)),
			hasCenter(toolType != J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW)
		{
			const XMFLOAT3 extents = JResourceManager::Instance().GetDefaultMeshGeometry(shape)->GetBoundingBox().Extents;
			shapeLength = JMathHelper::Vector3Length(extents) * 2;

			if (hasCenter)
			{
				JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();
				JMaterial* material = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"ArrowRootMaterial",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					dir));

				material->SetAlbedoColor(XMFLOAT4(0.7f, 0.7f, 0.7f, 0.7f));
				material->SetDebugMaterial(true);
				//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS);

				arrowCenterMaterial = Core::GetUserPtr(material);
			}

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
		void JEditorTransformTool::Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam)
		{
			bool isValid = selected.IsValid() && selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT;
			if (isValid)
			{
				if (!IsValid())
					CreateToolObject();
			}
			else
			{
				if(IsValid())
					DestroyToolObject();
			}

			if (!IsValid())
				return;

			JGameObject* gameObject = static_cast<JGameObject*>(selected.Get());
			UpdateArrowPosition(gameObject, cam);
			UpdateArrowDragging(gameObject, cam);	 
		}
		void JEditorTransformTool::UpdateArrowPosition(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{
			auto getNDCLam = [](const XMVECTOR v, const XMMATRIX m)
			{
				XMVECTOR clipSpaceV = XMVector3TransformCoord(v, m);
				XMFLOAT4 clipSpaceF;
				XMStoreFloat4(&clipSpaceF, clipSpaceV);
				return (clipSpaceV / clipSpaceF.w) * XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f) + XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f);
			};
			auto getViewLam = [](const XMVECTOR v, const XMMATRIX m, const float z)
			{
				return XMVector3TransformCoord((v - XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f)) * XMVectorSet(2.0f, 2.0f, 0.0f, 0.0f) * z, m);
			};
			const float camAspect = cam->GetAspect();
			ImVec2 wndSize = ImGui::GetWindowSize();
			const float fixedFactor = wndSize.x > wndSize.y ? wndSize.x * 0.075f : wndSize.y * 0.075f;
			JVector2<float> clipSpace = JVector2<float>(fixedFactor / wndSize.x, fixedFactor / wndSize.y) * 0.5f;

			const XMMATRIX worldViewM = XMMatrixMultiply(selected->GetTransform()->GetWorld(), cam->GetView());
			XMFLOAT4X4 worldViewF;
			XMStoreFloat4x4(&worldViewF, worldViewM);
			const float z = worldViewF._43;

			const float fovX = cam->GetFovX() * 0.5f;
			const float x = camAspect * clipSpace.x;
			const float finalX = x * z * fovX;

			const float fovY = cam->GetFovY() * 0.5f;
			const float y = camAspect * clipSpace.y;
			const float finalY = y * z * fovY;

			const BoundingBox bbox = arrow[0].arrow->GetRenderItem()->GetMesh()->GetBoundingBox();
			const float length = bbox.Extents.x > bbox.Extents.y ? bbox.Extents.x : bbox.Extents.y;
			transformArrowRoot->GetTransform()->SetScale(XMFLOAT3(finalX / length, finalY / length, finalX / length));
			
			JRenderItem* rItem = selected->GetRenderItem();
			if (rItem == nullptr)
			{ 
				XMFLOAT4X4 worldF;
				XMStoreFloat4x4(&worldF, selected->GetTransform()->GetWorld());			 
				transformArrowRoot->GetTransform()->SetPosition(XMFLOAT3(worldF._41, worldF._42, worldF._43));
			}
			else
				transformArrowRoot->GetTransform()->SetPosition(rItem->GetBoundingBox().Center);
		}
		void JEditorTransformTool::UpdateArrowDragging(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{
			const ImVec2 windowPos  = ImGui::GetWindowPos();
			const ImVec2 windowSize = ImGui::GetWindowSize();
 
			if (!isDraggingObject)
			{
				if (!JImGuiImpl::IsMouseInRect(windowPos, windowSize))
				{
					OffHovering();
					return;
				}
				 
				JGameObject* hitObj = SceneIntersect(Core::GetUserPtr(selected->GetOwnerScene()), 
					cam, 
					Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT);
				if (hitObj != nullptr)
				{
					for (uint i = 0; i < Constants::arrowCount; ++i)
					{
						if (arrow[i].arrow->GetGuid() == hitObj->GetGuid())
						{
							OnHovering(i);
							break;
						}
					}

					if (ImGui::IsMouseDown(0))
						OnDragging();
				}
				else
					OffHovering(); 
			}
			else
			{
				if (!ImGui::IsMouseDragging(0))
				{
					OffDragging();
					return;
				}
				if(IsEditable(selected))
					(transformUpdatePtr)(this, selected);
				preWorldMousePos = ImGui::GetMousePos();
			}
		}
		JEditorTransformTool::UpdateTransformT::Ptr JEditorTransformTool::GetUpdateTransformPtr(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType)noexcept
		{
			switch (toolType)
			{
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW:
				return JEditorTransformTool::UpdateSelectedPosition;
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW:
				return JEditorTransformTool::UpdateSelectedRotation;
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW:
				return JEditorTransformTool::UpdateSelectedScale;
			default:
				return nullptr;
			}
		}
		void JEditorTransformTool::UpdateSelectedPosition(JEditorTransformTool* tool, JGameObject* selected)noexcept
		{
			const float dPosFactor = 0.1f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowPos = transform->GetPosition();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preWorldMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				transform->SetPosition(XMFLOAT3(nowPos.x + mDelta.x, nowPos.y, nowPos.z));
			else if (tool->draggingIndex == 1)
				transform->SetPosition(XMFLOAT3(nowPos.x, nowPos.y + mDelta.y, nowPos.z));
			else if (tool->draggingIndex == 2)
				transform->SetPosition(XMFLOAT3(nowPos.x, nowPos.y, nowPos.z + mDelta.x)); 
			tool->SetModifiedBit(Core::GetUserPtr(selected->GetOwnerScene()), true);
		}
		void JEditorTransformTool::UpdateSelectedRotation(JEditorTransformTool* tool, JGameObject* selected)noexcept
		{ 
			const float dRotFactor = 0.5f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowRot = transform->GetRotation();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preWorldMousePos; 
			mDelta.x = std::clamp(mDelta.x, -dRotFactor, dRotFactor);

			if (tool->draggingIndex == 0)
				transform->SetRotation(XMFLOAT3(nowRot.x + mDelta.x, nowRot.y, nowRot.z));
			else if (tool->draggingIndex == 1)
				transform->SetRotation(XMFLOAT3(nowRot.x, nowRot.y + mDelta.x, nowRot.z));
			else if (tool->draggingIndex == 2)
				transform->SetRotation(XMFLOAT3(nowRot.x, nowRot.y, nowRot.z + mDelta.x));
			tool->SetModifiedBit(Core::GetUserPtr(selected->GetOwnerScene()), true);
		}
		void JEditorTransformTool::UpdateSelectedScale(JEditorTransformTool* tool, JGameObject* selected)noexcept
		{
			const float dPosFactor = 0.1f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowScale = transform->GetScale();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preWorldMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				transform->SetScale(XMFLOAT3(nowScale.x + mDelta.x, nowScale.y, nowScale.z));
			else if (tool->draggingIndex == 1)
				transform->SetScale(XMFLOAT3(nowScale.x, nowScale.y + mDelta.y, nowScale.z));
			else if (tool->draggingIndex == 2)
				transform->SetScale(XMFLOAT3(nowScale.x, nowScale.y, nowScale.z + mDelta.x));
			tool->SetModifiedBit(Core::GetUserPtr(selected->GetOwnerScene()), true);
		}
		void JEditorTransformTool::ActivateTool()noexcept
		{
			Activate();
		}
		void JEditorTransformTool::DeActivateTool()noexcept
		{
			DeActivate();
		}
		void JEditorTransformTool::DoActivate()noexcept
		{
			JActivatedInterface::DoActivate();
			CreateToolObject();
		}
		void JEditorTransformTool::DoDeActivate()noexcept
		{
			JActivatedInterface::DoDeActivate();
			DestroyToolObject();
		}
		void JEditorTransformTool::CreateToolObject()noexcept
		{  
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* newTransformArrowRoot = JGFI::Create(L"Transform Arrow Root", Core::MakeGuid(), flag, *debugRoot);

			//x y z 
			if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW)
			{ 
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 0, -90}, JVector3<float>{0, 0, 0}, JVector3<float>{90, 0, 0},
				};
				JVector3<float> pos[3]
				{
					JVector3<float>{0.11f, 0, 0}, JVector3<float>{0, 0.11f, 0}, JVector3<float>{0, 0, 0.11f},
				};
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos[i]);
			}
			else if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW)
			{ 
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 90, 0}, JVector3<float>{90, 0, 0}, JVector3<float>{0, 0, 0},
				};
				JVector3<float> pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos);
			}
			else
			{ 
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 0, -90}, JVector3<float>{0, 0, 0}, JVector3<float>{90, 0, 0},
				};
				JVector3<float> pos[3]
				{
					JVector3<float>{0.11f, 0, 0}, JVector3<float>{0, 0.11f, 0}, JVector3<float>{0, 0, 0.11f},
				};
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos[i]);
			}

			if (hasCenter)
			{
				JGameObject* newArrowCenter = JGFU::CreateShape(*newTransformArrowRoot, flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
				JRenderItem* arrowCenterRItem = newArrowCenter->GetRenderItem();
				arrowCenterRItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_UI);
				arrowCenterRItem->SetMaterial(0, arrowCenterMaterial.Get());
				newArrowCenter->GetTransform()->SetScale(XMFLOAT3(0.35f, 0.35f, 0.35f));
				arrowCenter = Core::GetUserPtr(newArrowCenter);
			}
			transformArrowRoot = Core::GetUserPtr(newTransformArrowRoot);

			OffHovering();
			OffDragging();
			SetValid(true);
		}
		void JEditorTransformTool::DestroyToolObject()noexcept
		{
			OffDragging();
			OffHovering();
			if (transformArrowRoot.IsValid())
				JGameObject::BeginDestroy(transformArrowRoot.Get());
			SetValid(false);
		}
		J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE JEditorTransformTool::GetToolType()const noexcept
		{
			return toolType;
		}
		void JEditorTransformTool::SetDebugRoot(Core::JUserPtr<JGameObject> debugRoot)
		{
			JEditorTransformTool::debugRoot = debugRoot;
			if (IsActivated())
			{
				DeActivate();
				Activate();
			}
		}
		void JEditorTransformTool::OnHovering(const int newArrowIndex)noexcept
		{
			if (hoveringIndex != newArrowIndex)
			{
				if (hoveringIndex != -1)
					arrow[hoveringIndex].OffHoveredColor();
				hoveringIndex = newArrowIndex;
				arrow[hoveringIndex].SetHoveredColor();
			}
		}
		void JEditorTransformTool::OffHovering()noexcept
		{
			if (hoveringIndex != -1)
				arrow[hoveringIndex].OffHoveredColor();
			hoveringIndex = -1;
		}
		void JEditorTransformTool::OnDragging()noexcept
		{
			isDraggingObject = true;
			draggingIndex = hoveringIndex;
			preWorldMousePos = ImGui::GetMousePos();
		}
		void JEditorTransformTool::OffDragging()noexcept
		{
			isDraggingObject = false;
			draggingIndex = -1;
		}
	}
}