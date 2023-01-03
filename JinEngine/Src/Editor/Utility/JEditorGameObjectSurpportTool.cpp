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
		bool JEditorGameObjectSurpportTool::IsEditable(JGameObject* obj)const noexcept
		{
			return !obj->HasFlag(OBJECT_FLAG_UNEDITABLE);
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
			JGameObject* gameObject = static_cast<JGameObject*>(selected.Get());
			UpdateArrowPosition(gameObject, cam);
			UpdateArrowDragging(gameObject, cam);
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
			const ImVec2 windowPos = ImGui::GetWindowPos();
			const ImVec2 windowSize = ImGui::GetWindowSize();
 
			if (!isDraggingObject)
			{
				if (!JImGuiImpl::IsMouseInRect(windowPos, windowSize))
				{
					OffHovering();
					return;
				}

				const ImVec2 cursorPos = JImGuiImpl::GetWorldCursorPos();
				const ImVec2 mousePos = ImGui::GetMousePos() - cursorPos;
				const ImVec2 camViewSize = ImVec2(cam->GetViewWidth(), cam->GetViewHeight());
				const float widthRate = (float)camViewSize.x / windowSize.x;
				const float heightRate = (float)camViewSize.y / windowSize.y;
				const ImVec2 finalMousePos = ImVec2(mousePos.x * widthRate, mousePos.y * heightRate);

				const XMFLOAT4X4 proj = cam->GetProj4x4f();
				const XMMATRIX invView = XMMatrixInverse(nullptr, cam->GetView());

				const float vx = ((2.0f * finalMousePos.x) / cam->GetViewWidth() - 1.0f) / proj(0, 0);
				const float vy = ((-2.0f * finalMousePos.y) / cam->GetViewHeight() + 1.0f) / proj(1, 1);
				const XMVECTOR rayOri = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invView);
				const XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(vx, vy, 1.0f, 0.0f), invView));

				JScene* scene = transformArrowRoot->GetOwnerScene();
				JGameObject* hitObj = scene->Intersect(Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT, Core::JRay{ rayOri, rayDir });
			
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
				preMousePos = ImGui::GetMousePos();
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
			const float dPosFactor = 0.05f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowPos = transform->GetPosition();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				transform->SetPosition(XMFLOAT3(nowPos.x + mDelta.x, nowPos.y, nowPos.z));
			else if (tool->draggingIndex == 1)
				transform->SetPosition(XMFLOAT3(nowPos.x, nowPos.y + mDelta.y, nowPos.z));
			else if (tool->draggingIndex == 2)
				transform->SetPosition(XMFLOAT3(nowPos.x, nowPos.y, nowPos.z + mDelta.x)); 
		}
		void JEditorTransformTool::UpdateSelectedRotation(JEditorTransformTool* tool, JGameObject* selected)noexcept
		{
			const float dRotFactor = 0.05f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowRot = transform->GetRotation();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preMousePos;
			mDelta.x = std::clamp(mDelta.x, -dRotFactor, dRotFactor);
			mDelta.y = std::clamp(-mDelta.y, -dRotFactor, dRotFactor);

			if (tool->draggingIndex == 0)
				transform->SetRotation(XMFLOAT3(nowRot.x + mDelta.x, nowRot.y, nowRot.z));
			else if (tool->draggingIndex == 1)
				transform->SetRotation(XMFLOAT3(nowRot.x, nowRot.y + mDelta.x, nowRot.z));
			else if (tool->draggingIndex == 2)
				transform->SetRotation(XMFLOAT3(nowRot.x, nowRot.y, nowRot.z + mDelta.x));
		}
		void JEditorTransformTool::UpdateSelectedScale(JEditorTransformTool* tool, JGameObject* selected)noexcept
		{
			const float dPosFactor = 0.05f;
			JTransform* transform = selected->GetTransform();
			XMFLOAT3 nowScale = transform->GetScale();
			ImVec2 nowMosePos = ImGui::GetMousePos();
			ImVec2 mDelta = nowMosePos - tool->preMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				transform->SetScale(XMFLOAT3(nowScale.x + mDelta.x, nowScale.y, nowScale.z));
			else if (tool->draggingIndex == 1)
				transform->SetScale(XMFLOAT3(nowScale.x, nowScale.y + mDelta.y, nowScale.z));
			else if (tool->draggingIndex == 2)
				transform->SetScale(XMFLOAT3(nowScale.x, nowScale.y, nowScale.z + mDelta.x));
		}
		bool JEditorTransformTool::IsValid()const noexcept
		{
			return transformArrowRoot.IsValid() && arrow[0].IsValid()
				&& arrow[1].IsValid()
				&& arrow[2].IsValid();
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
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* transformArrowRoot = JGFI::Create(L"Transform Arrow Root", Core::MakeGuid(), flag, *debugRoot);

			//x y z 
			if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW)
			{
				JVector3<float> scale = JVector3<float>{ 0.85f, 0.85f, 0.85f };
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 0, -90}, JVector3<float>{0, 0, 0}, JVector3<float>{90, 0, 0},
				};
				JVector3<float> pos[3]
				{
					JVector3<float>{0.11f, 0, 0}, JVector3<float>{0, 0.11f, 0}, JVector3<float>{0, 0, 0.11f},
				};
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(transformArrowRoot, shape, scale, rot[i], pos[i]);
			}
			else if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW)
			{
				JVector3<float> scale = JVector3<float>{ 0.85f, 0.85f, 0.85f };
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 90, 0}, JVector3<float>{90, 0, 0}, JVector3<float>{0, 0, 0},
				};
				JVector3<float> pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(transformArrowRoot, shape, scale, rot[i], pos);
			}
			else
			{
				JVector3<float> scale = JVector3<float>{ 0.85f, 0.85f, 0.85f };
				JVector3<float> rot[3]
				{
					JVector3<float>{0, 0, -90}, JVector3<float>{0, 0, 0}, JVector3<float>{90, 0, 0},
				};
				JVector3<float> pos[3]
				{
					JVector3<float>{0.11f, 0, 0}, JVector3<float>{0, 0.11f, 0}, JVector3<float>{0, 0, 0.11f},
				};
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(transformArrowRoot, shape, scale, rot[i], pos[i]);
			}

			if (hasCenter)
			{
				JGameObject* arrowCenter = JGFU::CreateShape(*transformArrowRoot, flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
				JRenderItem* arrowCenterRItem = arrowCenter->GetRenderItem();
				arrowCenterRItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_LAYER);
				arrowCenterRItem->SetMaterial(0, arrowCenterMaterial.Get());
				arrowCenter->GetTransform()->SetScale(XMFLOAT3(0.35f, 0.35f, 0.35f));
				JEditorTransformTool::arrowCenter = Core::GetUserPtr(arrowCenter);
			}
			JEditorTransformTool::transformArrowRoot = Core::GetUserPtr(transformArrowRoot);

			OffHovering();
			OffDragging();
		}
		void JEditorTransformTool::DoDeActivate()noexcept
		{
			JActivatedInterface::DoDeActivate();
			OffDragging();
			OffHovering();
			if (transformArrowRoot.IsValid())
				JGameObject::BeginDestroy(transformArrowRoot.Get());
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
			preMousePos = ImGui::GetMousePos();
		}
		void JEditorTransformTool::OffDragging()noexcept
		{
			isDraggingObject = false;
			draggingIndex = -1;
		}
	}
}