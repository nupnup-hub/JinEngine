#include"JEditorGameObjectSurpportTool.h"
#include"JEditorSceneImageInteraction.h"
#include"../Page/JEditorPageShareData.h"
#include"../Gui/JGui.h" 
#include"../../Object/Component/Camera/JCamera.h"  
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../Object/GameObject/JGameObject.h"  
#include"../../Object/GameObject/JGameObjectCreator.h"   
#include"../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../Object/Resource/Material/JMaterial.h" 
#include"../../Object/Resource/Material/JDefaultMaterialType.h" 
#include"../../Object/Resource/Scene/JScene.h" 
#include"../../Object/Resource/JResourceManager.h"  
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/Geometry/JRay.h"
#include"../../Core/Geometry/JBBox.h"
#include"../../Core/Math/JMathHelper.h" 

//Debug
//#include"../../Debug/JDebugTextOut.h"
#include "../../Core/Time/JStopWatch.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{		
		namespace Private
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
			//determine mouse delta direction by cam and arrow world position
			static JVector3<float> GetMouseDeltaDirFactor(JGameObject* tool, JCamera* cam)
			{ 
				const JMatrix4x4 camWorld = cam->GetTransform()->GetWorldMatrix4x4();
				const JMatrix4x4 arrowWorld = tool->GetTransform()->GetWorldMatrix4x4();
				const JVector3<float> dir = JVector3F(arrowWorld._41 - camWorld._41,
					arrowWorld._42 - camWorld._42,
					arrowWorld._43 - camWorld._43).Normalize();

				//In world coord
				//cam.z > arrow.z x dir 반전
				//cam.x < arrow.x z dir 반전(mouse left move => move z factor * -1)
				return XMFLOAT3(dir.z >= 0 ? 1 : -1, 1, dir.x >= 0 ? -1 : 1); 
			} 
			static JVector3<float> GetMidPosByPoint(const std::vector<JUserPtr<JGameObject>>& selected)
			{
				JVector3<float> p;
				for (const auto& data : selected)
					p += data->GetTransform()->GetWorldPosition();	 
				return p / selected.size();
			}
			static JVector3<float> GetMidPosByBBox(const std::vector<JUserPtr<JGameObject>>& selected)
			{
				Core::JBBox bbox;
				for (const auto& data : selected)
					bbox = Core::JBBox::Union(bbox, data->GetRenderItem()->GetBoundingBox());
				return bbox.Center();
			}
			static bool IsValidSelected(const JUserPtr<JGameObject>& selected)noexcept
			{
				return selected.IsValid();
			}
			static bool IsValidSelected(const std::vector<JUserPtr<JGameObject>>& selected)noexcept
			{ 
				for (const auto& data : selected)
				{ 
					if (data != nullptr)
						return true;
				}
				return false;
			}
		}

		bool JEditorGameObjectSurpportTool::IsEditable(JGameObject* obj)const noexcept
		{
			return !obj->HasFlag(OBJECT_FLAG_UNEDITABLE);
		}
		void JEditorTransformTool::Arrow::CreateMaterial(const JVector4<float> matColor)
		{
			JEditorTransformTool::Arrow::matColor = matColor;

			JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();
			JUserPtr<JMaterial> material = JICI::Create<JMaterial>(L"ArrowMaterial",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				JMaterial::GetDefaultFormatIndex(),
				dir);

			material->SetAlbedoColor(matColor);
			material->SetDebugMaterial(true);
			material->SetNonCulling(true);
			//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);

			JEditorTransformTool::Arrow::material = material;
		}
		void JEditorTransformTool::Arrow::Initialze(const JUserPtr<JGameObject>& debugRoot,
			const J_DEFAULT_SHAPE shape,
			const JVector3<float> initRotation,
			const JVector3<float> initMovePos)
		{ 
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JUserPtr<JGameObject> newArrow = JGCI::CreateShape(debugRoot, flag, shape);
			JUserPtr<JTransform> transform = newArrow->GetTransform();

			transform->SetRotation(initRotation);
			transform->SetPosition(initMovePos);

			JUserPtr<JRenderItem> rItem = newArrow->GetRenderItem();
			const uint subMeshCount = rItem->GetSubmeshCount();
			for (uint j = 0; j < subMeshCount; ++j)
				rItem->SetMaterial(j, material);
			rItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_UI);

			arrow = newArrow;
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
		void JEditorTransformTool::Arrow::SetSelectedColor()noexcept
		{
			material->SetAlbedoColor((matColor + Private::GetHoveredDeepFactor()));
		}
		void JEditorTransformTool::Arrow::OffSelectedColor()noexcept
		{ 
			material->SetAlbedoColor(matColor);
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
			mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(shape);
			 
			if (hasCenter)
			{
				JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();
				JUserPtr<JMaterial> material = JICI::Create<JMaterial>(L"ArrowRootMaterial",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					JMaterial::GetDefaultFormatIndex(),
					dir);

				material->SetAlbedoColor(JVector4<float>(0.85f, 0.85f, 0.85f, 0.7f));
				material->SetDebugMaterial(true);
				//material->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS);

				arrowCenterMaterial = material;
			}

			JVector4<float> color[arrowCount]
			{
				JVector4<float>{0.8f, 0, 0, 0.75f},
				JVector4<float>{0, 0.8f, 0, 0.75f},
				JVector4<float>{0, 0, 0.8f, 0.75f},
			};

			for (uint i = 0; i < arrowCount; ++i)
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
		void JEditorTransformTool::Update(const JUserPtr<JGameObject>& selected,
			const JUserPtr<JCamera>& cam,
			const JVector2<float>& sceneImageMinPoint,
			const bool canSelectToolObject)
		{
			UpdateStart();
			UpdateToolObject(Private::IsValidSelected(selected));
			if (!IsValid())
				return;
			
			std::vector<JUserPtr<JGameObject>> v{ selected };
			UpdateArrowPosition(Private::GetMidPosByPoint(v), cam);
			UpdateArrowDragging(v, cam, sceneImageMinPoint, canSelectToolObject);
		}
		void JEditorTransformTool::Update(const std::vector<JUserPtr<JGameObject>>& selected,
			const JUserPtr<JCamera>& cam, 
			const JVector2<float>& sceneImageMinPoint,
			const bool canSelectToolObject)
		{
			UpdateStart();
			UpdateToolObject(Private::IsValidSelected(selected));
			if (!IsValid())
				return;
			 
			UpdateArrowPosition(Private::GetMidPosByPoint(selected), cam);
			UpdateArrowDragging(selected, cam, sceneImageMinPoint, canSelectToolObject);
		}
		void JEditorTransformTool::UpdateStart()
		{
			uData.isLastUpdateSelected = false;
		}
		void JEditorTransformTool::UpdateToolObject(const bool isValidSelected)
		{
			if (isValidSelected)
			{
				if (!IsValid())
					CreateToolObject();
			}
			else
			{
				if (IsValid())
					DestroyToolObject();
			}
		}
		void JEditorTransformTool::UpdateArrowPosition(const JVector3<float>& posW, const JUserPtr<JCamera>& cam)
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
			 
			//scale factor
			//x = r * clipX * tan(x) * z;
			//y = r * clipY * tan(y) * z;
			//z = cam to selcted distance(cam view)
			const float camAspect = cam->GetAspect();
			const JVector2<float> wndSize = JGui::GetWindowSize();
			const float fixedFactor = wndSize.x > wndSize.y ? wndSize.x * 0.075f : wndSize.y * 0.075f;
			const JVector2<float> clipSpace = JVector2<float>(fixedFactor / wndSize.x, fixedFactor / wndSize.y) * 0.5f;

			//cam to seleceted distance
			const JVector3<float> posV = XMVector3Transform(posW.ToXmV(), cam->GetView());
			const float z = posV.z;
			 
			//clipSpace to world space
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
				const float centerRate = (arrowLength / centerLength) * Private::initCentetScalefactor;
				arrowCenter->GetTransform()->SetScale(XMFLOAT3(centerRate, centerRate, centerRate));
			} 

			//caution!
			//world pos를 지정해서 움직이기때문에 transformArrowRoot에 부모에 transform에 영향을 받을 수 있으므로
			//transformArrowRoot에 부모는 debugRoot으로 지정
			transformArrowRoot->GetTransform()->SetPosition(posW);
		}
		void JEditorTransformTool::UpdateArrowDragging(const std::vector<JUserPtr<JGameObject>>& selected,
			const JUserPtr<JCamera>& cam,
			const JVector2<float>& sceneImageMinPoint,
			const bool canSelectToolObject)
		{
			const JVector2<float> windowPos  = JGui::GetWindowPos();
			const JVector2<float> windowSize = JGui::GetWindowSize();

			if (!IsDraggingObject())
			{
				if (!JGui::IsMouseInRect(windowPos, windowSize))
				{
					OffHovering();
					return;
				}

				//bvh first intersect
				JUserPtr<JGameObject> hitObj = JEditorSceneImageInteraction::Intersect(cam->GetOwner()->GetOwnerScene(),
					cam,
					J_ACCELERATOR_LAYER::DEBUG_OBJECT,
					sceneImageMinPoint);
				if (hitObj != nullptr)
				{
					for (uint i = 0; i < arrowCount; ++i)
					{
						if (arrow[i].arrow->GetGuid() == hitObj->GetGuid())
						{
							OnHovering(i);
							break;
						}
					}
					 
					if (JGui::IsMouseDown(Core::J_MOUSE_BUTTON::LEFT) && canSelectToolObject)
						OnDragging();
				}
				else
					OffHovering();
			}
			else
			{
				if (!JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT))
				{
					OffDragging();
					return;
				}
				//if(IsEditable(selected.Get()))
				(transformUpdatePtr)(this, selected, cam);
				uData.preWorldMousePos = JGui::GetMousePos();
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
		void JEditorTransformTool::UpdateSelectedPosition(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept
		{ 
			const JVector3<float> deltaDir = Private::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam.Get());
			const JVector2<float> nowMosePos = JGui::GetMousePos();

			JVector2<float> delta = nowMosePos - tool->uData.preWorldMousePos;
			const float distance = tool->transformArrowRoot->GetTransform()->GetDistance(cam->GetTransform());
			delta.y = -delta.y;
			delta *= (distance / cam->GetFar()) * 1.5f;
			 
			JVector3<float> added = JVector3<float>::Zero();
			if (tool->uData.hoveringIndex == 0)
				added.x += delta.x * deltaDir.x;
			else if (tool->uData.hoveringIndex == 1)
				added.y += delta.y * deltaDir.y;
			else if (tool->uData.hoveringIndex == 2)
				added.z += delta.x * deltaDir.z;
			 
			for (const auto& data : selected)
			{
				auto t = data->GetTransform().Get();
				t->SetPosition(t->GetPosition() + added);
			}
			if (selected.size() > 0)
				tool->SetModifiedBit(selected[0]->GetOwnerScene(), true);
		}
		void JEditorTransformTool::UpdateSelectedRotation(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept
		{ 
			const float dRotFactor = 0.5f;   
			const JVector2<float> nowMosePos = JGui::GetMousePos();
			JVector2<float> delta = nowMosePos - tool->uData.preWorldMousePos;
			delta.x = std::clamp(delta.x, -dRotFactor, dRotFactor);

			JVector3<float> added = JVector3<float>::Zero();
			if (tool->uData.hoveringIndex == 0)
				added.x += delta.x;
			else if (tool->uData.hoveringIndex == 1)
				added.y += delta.x;
			else if (tool->uData.hoveringIndex == 2)
				added.z += delta.x;

			for (const auto& data : selected)
			{
				auto t = data->GetTransform().Get();
				t->SetRotation(t->GetRotation() + added);
			}
			if (selected.size() > 0)
				tool->SetModifiedBit(selected[0]->GetOwnerScene(), true);
		}
		void JEditorTransformTool::UpdateSelectedScale(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept
		{
			const float dPosFactor = 0.1f; 
			const JVector3<float> deltaDir = Private::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam.Get());
			const JVector2<float> nowMosePos = JGui::GetMousePos();
			JVector2<float> delta = nowMosePos - tool->uData.preWorldMousePos;
			delta.x = std::clamp(delta.x, -dPosFactor, dPosFactor);
			delta.y = std::clamp(-delta.y, -dPosFactor, dPosFactor);

			JVector3<float> added = JVector3<float>::Zero();
			if (tool->uData.hoveringIndex == 0)
				added.x += delta.x * deltaDir.x;
			else if (tool->uData.hoveringIndex == 1)
				added.y += delta.y * deltaDir.y;
			else if (tool->uData.hoveringIndex == 2)
				added.z += delta.x * deltaDir.z;

			for (const auto& data : selected)
			{
				auto t = data->GetTransform().Get();
				t->SetScale(t->GetScale() + added);
			}
			if(selected.size() > 0)
				tool->SetModifiedBit(selected[0]->GetOwnerScene(), true);
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
			DestroyToolObject();
			JActivatedInterface::DoDeActivate();
		}
		void JEditorTransformTool::CreateToolObject()noexcept
		{  
			//world기준 pos, rot, scale을 지정해서 Set을 호출하므로 부모 transform에 영향을 피하기위해
			//transformArrowRoot에 부모는 debugRoot으로 지정
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JUserPtr<JGameObject> newTransformArrowRoot = JICI::Create<JGameObject>(L"Transform Arrow Root", Core::MakeGuid(), flag, debugRoot);

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
				JUserPtr<JGameObject> newArrowCenter = JGCI::CreateShape(newTransformArrowRoot, flag, J_DEFAULT_SHAPE::SPHERE);
				JUserPtr<JRenderItem> arrowCenterRItem = newArrowCenter->GetRenderItem();
				arrowCenterRItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_UI);
				arrowCenterRItem->SetMaterial(0, arrowCenterMaterial);
				newArrowCenter->GetTransform()->SetScale(Private::GetInitCenterScaleFactor());
				arrowCenter = newArrowCenter;
			}
			transformArrowRoot = newTransformArrowRoot;
			 
			OffDragging();
			SetValid(true); 
		}
		void JEditorTransformTool::DestroyToolObject()noexcept
		{	 
			OffDragging(); 
			if (transformArrowRoot.IsValid())
				JGameObject::BeginDestroy(transformArrowRoot.Get());
			SetValid(false); 
		}
		J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE JEditorTransformTool::GetToolType()const noexcept
		{
			return toolType;
		}
		uint JEditorTransformTool::GetShapeLength()const noexcept
		{ 
			return JVector3<float>(mesh->GetBoundingBox().Extents).Length() * 2; 
		}
		void JEditorTransformTool::SetDebugRoot(JUserPtr<JGameObject> debugRoot)
		{
			JEditorTransformTool::debugRoot = debugRoot;
			if (IsActivated())
			{
				DeActivate();
				Activate();
			}
		}
		bool JEditorTransformTool::IsLastUpdateSelectedObject()const noexcept
		{
			return uData.isLastUpdateSelected;
		} 
		bool JEditorTransformTool::IsHoveringObject()const noexcept
		{
			return uData.hoveringIndex != invalidIndex;
		}
		bool JEditorTransformTool::IsDraggingObject()const noexcept
		{
			return uData.isDragging;
		}
		void JEditorTransformTool::OnDragging()noexcept
		{
			uData.isDragging = true;
			uData.isLastUpdateSelected = true;
			uData.preWorldMousePos = JGui::GetMousePos();
		}
		void JEditorTransformTool::OffDragging()noexcept
		{ 
			uData.isDragging = false;
		}
		void JEditorTransformTool::OnHovering(const int newArrowIndex)noexcept
		{
			if (uData.hoveringIndex != newArrowIndex)
			{
				if (uData.hoveringIndex != invalidIndex)
					arrow[uData.hoveringIndex].OffSelectedColor();
				uData.hoveringIndex = newArrowIndex;
				arrow[uData.hoveringIndex].SetSelectedColor();
			} 
		}
		void JEditorTransformTool::OffHovering()noexcept
		{
			if (uData.hoveringIndex != invalidIndex)
				arrow[uData.hoveringIndex].OffSelectedColor();
			uData.hoveringIndex = invalidIndex;
		}
	}
}