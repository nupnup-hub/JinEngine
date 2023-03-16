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
//#include"../../Debug/JDebugTextOut.h"
using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{		
		namespace Constants
		{
			static constexpr float initCentetScalefactor = 0.25f;
			static JVector3<float> GetInitCenterScaleFactor()noexcept
			{
				return JVector3<float>(initCentetScalefactor, initCentetScalefactor, initCentetScalefactor);
			}
			static constexpr float hoveredDeepFactor = 0.15f;
			static JVector4<float> GetHoveredDeepFactor()noexcept
			{
				return JVector4<float>(hoveredDeepFactor, hoveredDeepFactor, hoveredDeepFactor, hoveredDeepFactor);
			}
			static JVector3<float> GetMouseDeltaDirFactor(JGameObject* tool, JCamera* cam)
			{ 
				const XMFLOAT4X4 camWorld = cam->GetTransform()->GetWorld4x4f();
				const XMFLOAT4X4 arrowWorld = tool->GetTransform()->GetWorld4x4f();
				const XMFLOAT3 dir = JMathHelper::Vector3Normalize(XMFLOAT3(arrowWorld._41 - camWorld._41,
					arrowWorld._42 - camWorld._42,
					arrowWorld._43 - camWorld._43));
				return XMFLOAT3(dir.z >= 0 ? 1 : -1, 1, dir.x >= 0 ? -1 : 1);
			}
		}
		bool JEditorGameObjectSurpportTool::IsEditable(JGameObject* obj)const noexcept
		{
			return !obj->HasFlag(OBJECT_FLAG_UNEDITABLE);
		}
		JGameObject* JEditorGameObjectSurpportTool::SceneIntersect(Core::JUserPtr<JScene> scene, 
			Core::JUserPtr<JCamera> cam,
			Core::J_SPACE_SPATIAL_LAYER layer,
			const JVector2<float>& viewLocalPos) noexcept
		{
			if (!scene.IsValid() || !cam.IsValid())
				return nullptr;
			 
			const JVector2<float> windowPos = ImGui::GetWindowPos();
			const JVector2<float> windowSize = ImGui::GetWindowSize();
			if (!JImGuiImpl::IsMouseInRect(windowPos, windowSize))
				return nullptr;

			//Editor Window view port size = tab + menu + contents 
			const JVector2<float> localMousePos = ImGui::GetMousePos() -(ImGui::GetWindowPos() + viewLocalPos);
			const JVector2<float> camViewSize = JVector2<float>(cam->GetViewWidth(), cam->GetViewHeight());
			const float widthRate = (float)camViewSize.x / windowSize.x;
			const float heightRate = (float)camViewSize.y / windowSize.y;
			const JVector2<float> finalMousePos = JVector2<float>(localMousePos.x * widthRate, localMousePos.y * heightRate);

			const XMFLOAT4X4 proj = cam->GetProj4x4f();
			const XMMATRIX invView = XMMatrixInverse(nullptr, cam->GetView());

			const float vx = ((2.0f * finalMousePos.x) / cam->GetViewWidth() - 1.0f) / proj(0, 0);
			const float vy = ((-2.0f * finalMousePos.y) / cam->GetViewHeight() + 1.0f) / proj(1, 1);
			const XMVECTOR rayOri = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invView);
			const XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(vx, vy, 1.0f, 0.0f), invView));

			return scene->IntersectFirst(layer, Core::JRay{ rayOri, rayDir });
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
				rItem->SetMaterial(j, material);
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
			material->SetAlbedoColor((matColor + Constants::GetHoveredDeepFactor()).ConvertXMF());
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

				material->SetAlbedoColor(XMFLOAT4(0.85f, 0.85f, 0.85f, 0.7f));
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
		void JEditorTransformTool::Update(Core::JUserPtr<JGameObject> selected, Core::JUserPtr<JCamera> cam, const JVector2<float>& viewLocalPos)
		{
			bool isValid = selected.IsValid();
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
			UpdateArrowPosition(gameObject, cam.Get());
			UpdateArrowDragging(gameObject, cam.Get(), viewLocalPos);
		}
		void JEditorTransformTool::UpdateArrowPosition(JGameObject* selected, JCamera* cam)
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
			const JVector2<float> wndSize = ImGui::GetWindowSize();
			const float fixedFactor = wndSize.x > wndSize.y ? wndSize.x * 0.075f : wndSize.y * 0.075f;
			const JVector2<float> clipSpace = JVector2<float>(fixedFactor / wndSize.x, fixedFactor / wndSize.y) * 0.5f;

			const XMMATRIX worldViewM = XMMatrixMultiply(selected->GetTransform()->GetWorldMatrix(), cam->GetView());
			XMFLOAT4X4 worldViewF;
			XMStoreFloat4x4(&worldViewF, worldViewM);
			const float z = worldViewF._43;

			const float fovX = cam->GetFovX() * 0.5f;
			const float x = camAspect * clipSpace.x;
			const float finalX = x * z * fovX;

			const float fovY = cam->GetFovY() * 0.5f;
			const float y = camAspect * clipSpace.y;
			const float finalY = y * z * fovY;

			const BoundingBox arrowBBox = arrow[0].arrow->GetRenderItem()->GetMesh()->GetBoundingBox();
			const float arrowLength = arrowBBox.Extents.x > arrowBBox.Extents.y ? arrowBBox.Extents.x : arrowBBox.Extents.y;
			const XMFLOAT3 newScale = XMFLOAT3(finalX / arrowLength, finalY / arrowLength, finalX / arrowLength);
			transformArrowRoot->GetTransform()->SetScale(newScale);

			if (hasCenter)
			{
				const BoundingBox centerBBox = arrowCenter->GetRenderItem()->GetMesh()->GetBoundingBox();
				const float centerLength = centerBBox.Extents.x > centerBBox.Extents.y ? centerBBox.Extents.x : centerBBox.Extents.y;
				const float centerRate = (arrowLength / centerLength) * Constants::initCentetScalefactor;
				arrowCenter->GetTransform()->SetScale(XMFLOAT3(centerRate, centerRate, centerRate));
			} 

			JRenderItem* rItem = selected->GetRenderItem();
			if (rItem == nullptr)
			{ 
				XMFLOAT4X4 worldF;
				XMStoreFloat4x4(&worldF, selected->GetTransform()->GetWorldMatrix());			 
				transformArrowRoot->GetTransform()->SetPosition(XMFLOAT3(worldF._41, worldF._42 , worldF._43));
			}
			else
				transformArrowRoot->GetTransform()->SetPosition(rItem->GetBoundingBox().Center);
		}
		void JEditorTransformTool::UpdateArrowDragging(JGameObject* selected, JCamera* cam, const JVector2<float>& viewLocalPos)
		{
			const JVector2<float> windowPos  = ImGui::GetWindowPos();
			const JVector2<float> windowSize = ImGui::GetWindowSize();
 
			if (!isDraggingObject)
			{
				if (!JImGuiImpl::IsMouseInRect(windowPos, windowSize))
				{
					OffHovering();
					return;
				}
				 
				JGameObject* hitObj = SceneIntersect(Core::GetUserPtr(selected->GetOwnerScene()), 
					Core::GetUserPtr(cam), 
					Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT,
					viewLocalPos);
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
					(transformUpdatePtr)(this, selected, cam);
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
		void JEditorTransformTool::UpdateSelectedPosition(JEditorTransformTool* tool, JGameObject* selected, JCamera* cam)noexcept
		{
			static constexpr float dPosFactor = 0.1f; 
			const JVector3<float> delthFactor =Constants::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam);
			const JVector3<float> nowPos = selected->GetTransform()->GetPosition();
			const JVector2<float> nowMosePos = ImGui::GetMousePos();
			JVector2<float> mDelta = nowMosePos - tool->preWorldMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				selected->GetTransform()->SetPosition(XMFLOAT3(nowPos.x + mDelta.x * delthFactor.x, nowPos.y, nowPos.z));
			else if (tool->draggingIndex == 1)
				selected->GetTransform()->SetPosition(XMFLOAT3(nowPos.x, nowPos.y + mDelta.y * delthFactor.y, nowPos.z));
			else if (tool->draggingIndex == 2)
				selected->GetTransform()->SetPosition(XMFLOAT3(nowPos.x, nowPos.y, nowPos.z + mDelta.x * delthFactor.z));
			tool->SetModifiedBit(Core::GetUserPtr(selected->GetOwnerScene()), true);
		}
		void JEditorTransformTool::UpdateSelectedRotation(JEditorTransformTool* tool, JGameObject* selected, JCamera* cam)noexcept
		{ 
			const float dRotFactor = 0.5f;  
			const JVector3<float> nowRot = selected->GetTransform()->GetRotation();
			const JVector2<float> nowMosePos = ImGui::GetMousePos();
			JVector2<float> mDelta = nowMosePos - tool->preWorldMousePos;
			mDelta.x = std::clamp(mDelta.x, -dRotFactor, dRotFactor);

			if (tool->draggingIndex == 0)
				selected->GetTransform()->SetRotation(XMFLOAT3(nowRot.x + mDelta.x, nowRot.y, nowRot.z));
			else if (tool->draggingIndex == 1)
				selected->GetTransform()->SetRotation(XMFLOAT3(nowRot.x, nowRot.y + mDelta.x, nowRot.z));
			else if (tool->draggingIndex == 2)
				selected->GetTransform()->SetRotation(XMFLOAT3(nowRot.x, nowRot.y, nowRot.z + mDelta.x));
			tool->SetModifiedBit(Core::GetUserPtr(selected->GetOwnerScene()), true);
		}
		void JEditorTransformTool::UpdateSelectedScale(JEditorTransformTool* tool, JGameObject* selected, JCamera* cam)noexcept
		{
			const float dPosFactor = 0.1f; 
			const JVector3<float> delthFactor = Constants::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam);
			const JVector3<float> nowScale = selected->GetTransform()->GetScale();
			const JVector2<float> nowMosePos = ImGui::GetMousePos();
			JVector2<float> mDelta = nowMosePos - tool->preWorldMousePos;
			mDelta.x = std::clamp(mDelta.x, -dPosFactor, dPosFactor);
			mDelta.y = std::clamp(-mDelta.y, -dPosFactor, dPosFactor);

			if (tool->draggingIndex == 0)
				selected->GetTransform()->SetScale(XMFLOAT3(nowScale.x + mDelta.x * delthFactor.x, nowScale.y, nowScale.z));
			else if (tool->draggingIndex == 1)
				selected->GetTransform()->SetScale(XMFLOAT3(nowScale.x, nowScale.y + mDelta.y * delthFactor.y, nowScale.z));
			else if (tool->draggingIndex == 2)
				selected->GetTransform()->SetScale(XMFLOAT3(nowScale.x, nowScale.y, nowScale.z + mDelta.x * delthFactor.z));
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
				JVector3<float> pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos);
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
				JVector3<float> pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos);
			}

			if (hasCenter)
			{
				JGameObject* newArrowCenter = JGFU::CreateShape(*newTransformArrowRoot, flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
				JRenderItem* arrowCenterRItem = newArrowCenter->GetRenderItem();
				arrowCenterRItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_UI);
				arrowCenterRItem->SetMaterial(0, arrowCenterMaterial);
				newArrowCenter->GetTransform()->SetScale(Constants::GetInitCenterScaleFactor().ConvertXMF());
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