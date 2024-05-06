/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JEditorGameObjectSurpportTool.h"
#include"JEditorSceneImageInteraction.h"
#include"../Page/JEditorPageShareData.h"
#include"../Gui/JGui.h" 
#include"../../Object/Component/Camera/JCamera.h"  
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../Object/Component/Light/JPointLight.h"
#include"../../Object/Component/Light/JSpotLight.h" 
#include"../../Object/Component/Light/JRectLight.h" 
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
//#include"../../Develop/Debug/JDevelopDebug.h"  

using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static constexpr float initCentetScalefactor = 0.25f;
			static JVector3F GetInitCenterScaleFactor()noexcept
			{
				return JVector3F(initCentetScalefactor, initCentetScalefactor, initCentetScalefactor);
			}
			static constexpr float hoveredDeepFactor = 0.15f;
			static JVector4F GetHoveredDeepFactor()noexcept
			{
				return JVector4F(hoveredDeepFactor, hoveredDeepFactor, hoveredDeepFactor, hoveredDeepFactor);
			}
			//determine mouse delta direction by cam and arrow world position
			static JVector3F GetMouseDeltaDirFactor(JGameObject* tool, JCamera* cam)
			{
				const JMatrix4x4 camWorld = cam->GetTransform()->GetWorldMatrix();
				const JMatrix4x4 arrowWorld = tool->GetTransform()->GetWorldMatrix();
				const JVector3F dir = JVector3F(arrowWorld._41 - camWorld._41,
					arrowWorld._42 - camWorld._42,
					arrowWorld._43 - camWorld._43).Normalize();

				//In world coord
				//cam.z > arrow.z x dir 반전
				//cam.x < arrow.x z dir 반전(mouse left move => move z factor * -1)
				return XMFLOAT3(dir.z >= 0 ? 1 : -1, 1, dir.x >= 0 ? -1 : 1);
			}
			static JVector3F GetMidPosByPoint(const JUserPtr<JGameObject>& selected)
			{
				return selected->GetTransform()->GetWorldPosition();
			}
			static JVector3F GetMidPosByPoint(const std::vector<JUserPtr<JGameObject>>& selected)
			{
				JVector3F p = JVector3F::Zero();
				for (const auto& data : selected)
					p += data->GetTransform()->GetWorldPosition();
				return p / selected.size();
			}
			static JVector3F GetMidPosByBBox(const std::vector<JUserPtr<JGameObject>>& selected)
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
		JVector2F JEditorGameObjectSurpportTool::CalUiScale(const JVector3F& posW, const JVector2F fixedScale, const JUserPtr<JCamera>& cam)
		{
			//scale factor
			//x = r * clipX * tan(x) * z;
			//y = r * clipY * tan(y) * z;
			//z = cam to selcted distance(cam view)

			const JVector2F wndSize = JGui::GetWindowSize();
			const JVector2F clipSpace = (fixedScale / wndSize) * 0.5f;

			//cam to seleceted distance
			const JVector3F posV = XMVector3Transform(posW.ToXmV(), cam->GetView());
			const float z = posV.z;

			//clipSpace to world space
			const float camAspect = cam->GetAspect();

			const float fovX = cam->GetFovX() * 0.5f;
			const float x = camAspect * clipSpace.x;
			const float finalX = x * z * fovX;

			const float fovY = cam->GetFovY() * 0.5f;
			const float y = camAspect * clipSpace.y;
			const float finalY = y * z * fovY;

			return JVector2F(finalX, finalY);
		}

		void JEditorTransformTool::Arrow::CreateMaterial(const JVector4F matColor)
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
			const JVector3F initRotation,
			const JVector3F initMovePos)
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

		JVector3F JEditorTransformTool::UpdateData::InvalidPos()const noexcept
		{
			return JVector3F::PositiveInfV();
		}
		void JEditorTransformTool::UpdateData::Clear()
		{
			lastPos = InvalidPos();
			lastSelected.Clear();
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
			const JVector2F& sceneImageScreenMinPoint,
			const bool canSelectToolObject)
		{
			std::vector<JUserPtr<JGameObject>> vec{ selected };
			Update(selected, cam, sceneImageScreenMinPoint, canSelectToolObject);
		}
		void JEditorTransformTool::Update(const std::vector<JUserPtr<JGameObject>>& selected,
			const JUserPtr<JCamera>& cam,
			const JVector2F& sceneImageScreenMinPoint,
			const bool canSelectToolObject)
		{
			UpdateStart();
			if (!IsValid() || !Private::IsValidSelected(selected))
			{
				if (IsDragging())
					OffDragging();
				if (IsHovering())
					OffHovering();
				if (cam != nullptr && !(uData.lastPos == uData.InvalidPos()) && uData.lastSelected != nullptr)
				{
					JVector3F midPos = Private::GetMidPosByPoint(uData.lastSelected);
					UpdateArrowPosition(midPos, cam);
					uData.lastPos = midPos;
				}
				return;
			}

			JVector3F midPos = Private::GetMidPosByPoint(selected);
			UpdateArrowPosition(midPos, cam);
			UpdateArrowDragging(selected, cam, sceneImageScreenMinPoint, canSelectToolObject);
			uData.lastPos = midPos;
			if (selected.size() > 0)
				uData.lastSelected = selected[0];
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
		void JEditorTransformTool::UpdateArrowPosition(const JVector3F& posW, const JUserPtr<JCamera>& cam)
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

			const JVector2F wndSize = JGui::GetWindowSize();
			const float fixedFactor = wndSize.x > wndSize.y ? wndSize.x * 0.075f : wndSize.y * 0.075f;

			const JVector2F sizeRate = CalUiScale(posW, JVector2F(fixedFactor, fixedFactor), cam);
			const BoundingBox arrowBBox = arrow[0].arrow->GetRenderItem()->GetMesh()->GetBoundingBox();
			const float arrowLength = arrowBBox.Extents.x > arrowBBox.Extents.y ? arrowBBox.Extents.x : arrowBBox.Extents.y;
			const JVector3F newScale = JVector3F(sizeRate.x / arrowLength, sizeRate.y / arrowLength, sizeRate.x / arrowLength);

			if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW)
				transformArrowRoot->GetTransform()->SetScale(newScale + JVector3F(2, 2, 2));
			else
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
		void JEditorTransformTool::UpdateArrowDragging(const JUserPtr<JGameObject>& selected,
			const JUserPtr<JCamera>& cam,
			const JVector2F& sceneImageScreenMinPoint,
			const bool canSelectToolObject)
		{
			std::vector<JUserPtr<JGameObject>> v{ selected };
			UpdateArrowDragging(v, cam, sceneImageScreenMinPoint, canSelectToolObject);
		}
		void JEditorTransformTool::UpdateArrowDragging(const std::vector<JUserPtr<JGameObject>>& selected,
			const JUserPtr<JCamera>& cam,
			const JVector2F& sceneImageScreenMinPoint,
			const bool canSelectToolObject)
		{
			const JVector2F windowPos = JGui::GetWindowPos();
			const JVector2F windowSize = JGui::GetWindowSize();

			if (!IsDragging())
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
					sceneImageScreenMinPoint);
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
			const JVector3F deltaDir = Private::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam.Get());
			const JVector2F nowMosePos = JGui::GetMousePos();

			JVector2F delta = nowMosePos - tool->uData.preWorldMousePos;
			const float distance = tool->transformArrowRoot->GetTransform()->GetDistance(cam->GetTransform());
			delta.y = -delta.y;
			delta *= (distance / cam->GetFar()) * 1.5f;

			JVector3F added = JVector3F::Zero();
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
			const JVector2F nowMosePos = JGui::GetMousePos();
			JVector2F delta = nowMosePos - tool->uData.preWorldMousePos;
			delta.x = std::clamp(delta.x, -dRotFactor, dRotFactor);

			JVector3F added = JVector3F::Zero();
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
			const JVector3F deltaDir = Private::GetMouseDeltaDirFactor(tool->transformArrowRoot.Get(), cam.Get());
			const JVector2F nowMosePos = JGui::GetMousePos();
			JVector2F delta = nowMosePos - tool->uData.preWorldMousePos;
			delta.x = std::clamp(delta.x, -dPosFactor, dPosFactor);
			delta.y = std::clamp(-delta.y, -dPosFactor, dPosFactor);

			JVector3F added = JVector3F::Zero();
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
			if (selected.size() > 0)
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
			uData.Clear();
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
				JVector3F rot[3]
				{
					JVector3F{0, 0, -90}, JVector3F{0, 0, 0}, JVector3F{90, 0, 0},
				};
				JVector3F pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos);
			}
			else if (toolType == J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW)
			{
				JVector3F rot[3]
				{
					JVector3F{0, 90, 0}, JVector3F{90, 0, 0}, JVector3F{0, 0, 0},
				};
				JVector3F pos{ 0, 0, 0 };
				for (uint i = 0; i < 3; ++i)
					arrow[i].Initialze(newTransformArrowRoot, shape, rot[i], pos);
			}
			else
			{
				JVector3F rot[3]
				{
					JVector3F{0, 0, -90}, JVector3F{0, 0, 0}, JVector3F{90, 0, 0},
				};
				JVector3F pos{ 0, 0, 0 };
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
			return JVector3F(mesh->GetBoundingBox().Extents).Length() * 2;
		}
		JUserPtr<JGameObject> JEditorTransformTool::GetLastSelected()const noexcept
		{
			return uData.lastSelected;
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
		bool JEditorTransformTool::IsLastUpdateSelected()const noexcept
		{
			return uData.isLastUpdateSelected;
		}
		bool JEditorTransformTool::IsHovering()const noexcept
		{
			return uData.hoveringIndex != invalidIndex;
		}
		bool JEditorTransformTool::IsDragging()const noexcept
		{
			return uData.isDragging;
		}
		bool JEditorTransformTool::IsHitDebugObject(const JUserPtr<JCamera>& cam, const JVector2F& sceneImageScreenMinPoint)const noexcept
		{
			JUserPtr<JGameObject> hitObj = JEditorSceneImageInteraction::Intersect(cam->GetOwner()->GetOwnerScene(),
				cam,
				J_ACCELERATOR_LAYER::DEBUG_OBJECT,
				sceneImageScreenMinPoint);
			if (hitObj != nullptr)
			{
				for (uint i = 0; i < arrowCount; ++i)
				{
					if (arrow[i].arrow->GetGuid() == hitObj->GetGuid())
						return true;
				}
			}
			return false;
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

		JEditorGeometryTool::FrustumView::FrustumView(const JUserPtr<JCamera>& cam, const JUserPtr<JGameObject>& parent)
		{
			if (cam != nullptr && parent != nullptr && root == nullptr)
			{
				targetCam = cam;
				root = JICI::Create<JGameObject>(targetCam->GetName() + L" Debug Frustum Root",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					parent);

				nearFrustum = JGCI::CreateDebugLineShape(root,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::BOUNDING_FRUSTUM,
					J_DEFAULT_MATERIAL::DEBUG_LINE_RED,
					true);

				farFrustum = JGCI::CreateDebugLineShape(root,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::BOUNDING_FRUSTUM,
					J_DEFAULT_MATERIAL::DEBUG_LINE_RED,
					true); 
			}
		}
		JEditorGeometryTool::FrustumView::~FrustumView()
		{ 
			Clear();
		}
		void JEditorGeometryTool::FrustumView::Clear()
		{
			if (root.IsValid())
				JGameObject::BeginDestroy(root.Release());
		}
		void JEditorGeometryTool::FrustumView::Update()
		{
			if (!IsValid())
				return;

			JVector3F nearScale;
			JVector3F farScale;
			JUserPtr<JTransform> tarTransform = nullptr;
			if (targetCam != nullptr)
			{
				const float camNearWidth = targetCam->GetNearViewWidth();
				const float camNearHeight = targetCam->GetNearViewHeight();
				const float camFarWidth = targetCam->GetFarViewWidth();
				const float camFarHeight = targetCam->GetFarViewHeight();
				const float camNear = targetCam->GetNear();
				const float camFar = targetCam->GetFar();
				const float minimizeFactor = 1.0f / 32.0f;

				nearScale = JVector3<float>(camNearWidth, camNearHeight, camNear);
				farScale = JVector3<float>(camFarWidth * minimizeFactor, camFarHeight * minimizeFactor, camFar * minimizeFactor);
				tarTransform = targetCam->GetTransform();
			}
			if (tarTransform == nullptr)
				return;

			JVector3F worldP = tarTransform->GetWorldPosition();
			JVector3F worldR = tarTransform->GetWorldRotation();

			nearFrustum->GetTransform()->SetScale(nearScale);
			nearFrustum->GetTransform()->SetRotation(worldR);
			nearFrustum->GetTransform()->SetPosition(worldP);

			farFrustum->GetTransform()->SetScale(farScale);
			farFrustum->GetTransform()->SetRotation(worldR);
			farFrustum->GetTransform()->SetPosition(worldP);
		}
		size_t JEditorGeometryTool::FrustumView::GetTargetGuid()const noexcept
		{
			return targetCam->GetGuid();
		}
		Core::JTypeInfo& JEditorGeometryTool::FrustumView::GetTargetTypeInfo() const noexcept
		{
			return targetCam->GetTypeInfo();
		}
		void JEditorGeometryTool::FrustumView::SetMaterial(const JUserPtr<JMaterial>& mat)
		{
			nearFrustum->GetRenderItem()->SetMaterial(0, mat);
			farFrustum->GetRenderItem()->SetMaterial(0, mat);
		}
		bool JEditorGeometryTool::FrustumView::IsValid()const noexcept
		{
			return root != nullptr && nearFrustum != nullptr && farFrustum != nullptr && (targetCam != nullptr && targetCam->IsActivated());
		}

		JEditorGeometryTool::SphereView::SphereView(const JUserPtr<JPointLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit != nullptr && parent != nullptr && root == nullptr)
			{
				targetPoint = lit;
				root = JICI::Create<JGameObject>(lit->GetName() + L" Debug Sphere Root",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					parent);

				xyCircle = JGCI::CreateDebugShape(root, L"xy circle", OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::CIRCLE, J_DEFAULT_MATERIAL::DEBUG_YELLOW);
				xzCircle = JGCI::CreateDebugShape(root, L"xz circle", OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::CIRCLE, J_DEFAULT_MATERIAL::DEBUG_YELLOW);
				yzCircle = JGCI::CreateDebugShape(root, L"yz circle", OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::CIRCLE, J_DEFAULT_MATERIAL::DEBUG_YELLOW);
			}
		}
		JEditorGeometryTool::SphereView::~SphereView()
		{
			Clear();
		}
		void JEditorGeometryTool::SphereView::Clear()
		{
			if (root.IsValid())
				JGameObject::BeginDestroy(root.Release());
		}
		void JEditorGeometryTool::SphereView::Update()
		{
			JVector3F worldP;
			JVector3F worldR;
			float radius = 0;

			JUserPtr<JTransform> transform = nullptr;

			if (targetPoint != nullptr)
			{
				transform = targetPoint->GetOwner()->GetTransform();
				radius = targetPoint->GetRange();
			}
			if (transform == nullptr)
				return;

			worldP = transform->GetWorldPosition();
			worldR = transform->GetWorldRotation();

			float meshRadius = xyCircle->GetRenderItem()->GetMesh()->GetBoundingSphereRadius();
			float rate = 1.0f / meshRadius;
			float xyScale = radius * rate;
			//circle x = n, y = m,  z = 0
			xyCircle->GetTransform()->SetScale(JVector3F(xyScale, xyScale, 1.0f));
			xyCircle->GetTransform()->SetRotation(worldR);
			xyCircle->GetTransform()->SetPosition(worldP);

			xzCircle->GetTransform()->SetScale(JVector3F(xyScale, xyScale, 1.0f));
			xzCircle->GetTransform()->SetRotation(JVector3F(90, 0, 0) + worldR);
			xzCircle->GetTransform()->SetPosition(worldP);

			yzCircle->GetTransform()->SetScale(JVector3F(xyScale, xyScale, 1.0f));
			yzCircle->GetTransform()->SetRotation(JVector3F(0, 90, 0) + worldR);
			yzCircle->GetTransform()->SetPosition(worldP);
		}
		size_t JEditorGeometryTool::SphereView::GetTargetGuid()const noexcept
		{
			return targetPoint->GetGuid();
		}
		Core::JTypeInfo& JEditorGeometryTool::SphereView::GetTargetTypeInfo() const noexcept
		{
			return targetPoint->GetTypeInfo();
		}
		void JEditorGeometryTool::SphereView::SetMaterial(const JUserPtr<JMaterial>& mat)
		{
			xyCircle->GetRenderItem()->SetMaterial(0, mat);
			xzCircle->GetRenderItem()->SetMaterial(0, mat);
			yzCircle->GetRenderItem()->SetMaterial(0, mat);
		}
		bool JEditorGeometryTool::SphereView::IsValid()const noexcept
		{
			return root != nullptr && xyCircle != nullptr && xzCircle != nullptr && yzCircle != nullptr && targetPoint != nullptr && targetPoint->IsActivated() &&
				targetPoint->GetOwner()->IsSelected();
		}

		JEditorGeometryTool::ConeView::ConeView(const JUserPtr<JSpotLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit != nullptr && parent != nullptr && root == nullptr)
			{
				targetSpot = lit;
				root = JICI::Create<JGameObject>(lit->GetName() + L" Debug Spot Root",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					parent);

				boundingCone = JGCI::CreateDebugShape(root, L"bounding cone_L",
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::BOUNDING_CONE_LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW, false, true);
			}
		}
		JEditorGeometryTool::ConeView::~ConeView()
		{
			Clear();
		}
		void JEditorGeometryTool::ConeView::Clear()
		{
			if (root.IsValid())
				JGameObject::BeginDestroy(root.Release());
		}
		void JEditorGeometryTool::ConeView::Update()
		{
			JVector3F dirction;
			float range = 0;
			float outAngle = 0;

			JUserPtr<JTransform> transform = nullptr;
			if (targetSpot != nullptr)
			{
				transform = targetSpot->GetOwner()->GetTransform();
				dirction = targetSpot->GetDirection();
				range = targetSpot->GetRange();
				outAngle = targetSpot->GetOuterConeAngle();
			}
			if (transform == nullptr)
				return;

			//bounding
			float radius = 0.5f; 
			float bottomRadius = range * tan(outAngle);

			//boundingCone face z+
			JMatrix4x4 world;
			world.StoreXM(targetSpot->GetMeshWorldM());
			boundingCone->GetTransform()->SetTransform(world); 
		}
		size_t JEditorGeometryTool::ConeView::GetTargetGuid()const noexcept
		{
			return targetSpot->GetGuid();
		}
		Core::JTypeInfo& JEditorGeometryTool::ConeView::GetTargetTypeInfo() const noexcept
		{
			return targetSpot->GetTypeInfo();
		}
		void JEditorGeometryTool::ConeView::SetMaterial(const JUserPtr<JMaterial>& mat)
		{
			boundingCone->GetRenderItem()->SetMaterial(0, mat);
		}
		bool JEditorGeometryTool::ConeView::IsValid()const noexcept
		{
			return root != nullptr && boundingCone != nullptr && targetSpot != nullptr && targetSpot->IsActivated() && targetSpot->GetOwner()->IsSelected();
		}

		JEditorGeometryTool::RectView::RectView(const JUserPtr<JRectLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit != nullptr && parent != nullptr && root == nullptr)
			{
				targetRect = lit;
				root = JICI::Create<JGameObject>(lit->GetName() + L" Debug Rect Root",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					parent);

				for (int i = 0; i < 4; ++i)
					innerLine[i] = JGCI::CreateDebugLineShape(root, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW, false);
				for (int i = 0; i < 4; ++i)
					edgeLine[i] = JGCI::CreateDebugLineShape(root, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW, false);
				for (int i = 0; i < 4; ++i)
					outerLine[i] = JGCI::CreateDebugLineShape(root, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW, false);
			}
		}
		void JEditorGeometryTool::RectView::Clear()
		{
			if (root.IsValid())
				JGameObject::BeginDestroy(root.Release());
		}
		void JEditorGeometryTool::RectView::Update()
		{
			JVector3F worldP;
			JVector3F worldR;
			XMVECTOR worldQ;
			JVector2F areaSize;
			JUserPtr<JTransform> transform = nullptr;
			if (targetRect != nullptr)
			{
				transform = targetRect->GetOwner()->GetTransform();
				areaSize = targetRect->GetAreaSize();
			}
			if (transform == nullptr)
				return;

			worldP = transform->GetWorldPosition();
			worldQ = transform->GetWorldQuaternion().ToXmV();
			worldR = JMathHelper::ToEulerAngle(worldQ).ToXmV();

			float length = innerLine[0]->GetRenderItem()->GetMesh()->GetBoundingBoxExtent().y * 2;
			float rate = 1.0f / length;

			//r - l - u - b
			JVector3F innerScale[4]{ JVector3F(1.0f, rate * areaSize.y, 1.0f) , JVector3F(1.0f, rate * areaSize.y, 1.0f) , JVector3F(1.0f, rate * areaSize.x, 1.0f) , JVector3F(1.0f, rate * areaSize.x, 1.0f) };
			JVector3F innerRotation[4]{ JVector3F(0, 0, 0), JVector3F(0, 0, 0), JVector3F(0, 0, 90), JVector3F(0, 0, 90) };
			JVector3F innerPosition[4]{ JVector3F(areaSize.x * 0.5f, 0, 0), JVector3F(-areaSize.x * 0.5f, 0, 0), JVector3F(0, areaSize.y * 0.5f, 0), JVector3F(0, -areaSize.y * 0.5f, 0) };

			for (int i = 0; i < 4; ++i)
			{
				innerLine[i]->GetTransform()->SetScale(innerScale[i]);
				innerLine[i]->GetTransform()->SetRotation(worldR + innerRotation[i]);
				innerLine[i]->GetTransform()->SetPosition(worldP + XMVector3Rotate(innerPosition[i].ToXmV(), worldQ));
			}

			float barnAngle = targetRect->GetBarndoorAngle() * JMathHelper::DegToRad;
			float barnCosAngle = std::cos(barnAngle) * std::cos(barnAngle);
			float barnSinAngle = 1 - barnCosAngle;
			float barnLength = targetRect->GetBarndoorLength();
			float halfBarnLength = barnLength * 0.5f;

			XMVECTOR worldRight = transform->GetWorldRight().ToXmV();
			XMVECTOR worldUp = transform->GetWorldUp().ToXmV();
			XMVECTOR worldFront = transform->GetWorldFront().ToXmV();

			float barnConerRate = barnCosAngle * barnLength;
			float barnFrontRate = barnSinAngle * barnLength;

			JVector3F outerPositionOffset[4]
			{
				worldRight * barnConerRate + worldFront * barnFrontRate,
				 -worldRight * barnConerRate + worldFront * barnFrontRate,
				worldUp * barnConerRate + worldFront * barnFrontRate,
				-worldUp * barnConerRate + worldFront * barnFrontRate
			};
			for (int i = 0; i < 4; ++i)
			{
				outerLine[i]->GetTransform()->SetScale(innerScale[i] + JVector3F(0, barnConerRate * 2 * rate, 0));
				outerLine[i]->GetTransform()->SetRotation(worldR + innerRotation[i]);
				outerLine[i]->GetTransform()->SetPosition(worldP + XMVector3Rotate(innerPosition[i].ToXmV(), worldQ) + outerPositionOffset[i]);
			}
			float barnH = sqrt(barnLength * barnLength + barnLength * barnLength);
			float lineLength = barnSinAngle * barnLength + barnCosAngle * barnH;

			float linePosX = barnLength * 0.5f * barnCosAngle;
			float linePosY = barnLength * 0.5f * barnCosAngle;
			float linePosZ = barnLength * 0.5f * barnSinAngle;

			//R L U D 
			//Top-right, bottom-left, top-left, bottm-right
			JVector3F areaSizeOffset[4]
			{
				worldUp * areaSize.y * 0.5f,
				-worldUp * areaSize.y * 0.5f,
				-worldRight * areaSize.x * 0.5f,
				worldRight * areaSize.x * 0.5f
			}; 
			JVector3F linePosition[4]
			{
				JVector3F::Right() * linePosX + JVector3F::Up() * linePosY + JVector3F::Front() * linePosZ,
				JVector3F::Right() * -linePosX - JVector3F::Up() * linePosY + JVector3F::Front() * linePosZ,
				JVector3F::Right() * -linePosX + JVector3F::Up() * linePosY + JVector3F::Front() * linePosZ,
				JVector3F::Right()* linePosX - JVector3F::Up() * linePosY + JVector3F::Front() * linePosZ
			};
			for (int i = 0; i < 4; ++i)
			{
				auto innerPos = innerLine[i]->GetTransform()->GetWorldPosition() + areaSizeOffset[i];
				auto linePos = innerLine[i]->GetTransform()->GetWorldPosition() + areaSizeOffset[i] + XMVector3Rotate(linePosition[i].ToXmV(), worldQ);

				auto srcPos = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
				auto destPos = XMVector3Normalize(linePosition[i].ToXmV());

				auto axis = XMVector3Normalize(XMVector3Cross(srcPos, destPos));
				auto betAngle = XMVector3AngleBetweenNormals(srcPos, destPos);
				auto q =XMQuaternionRotationAxis(axis, XMVectorGetX(betAngle));
				  
				edgeLine[i]->GetTransform()->SetScale(JVector3F(1, (linePos - innerPos).Length() * 2, 1));
				edgeLine[i]->GetTransform()->SetRotation(JMathHelper::ToEulerAngle(DirectX::XMQuaternionMultiply(q, worldQ)));
				edgeLine[i]->GetTransform()->SetPosition(linePos);
			} 
		}
		JEditorGeometryTool::RectView::~RectView()
		{
			Clear();
		}
		size_t JEditorGeometryTool::RectView::GetTargetGuid()const noexcept
		{
			return targetRect->GetGuid();
		}
		Core::JTypeInfo& JEditorGeometryTool::RectView::GetTargetTypeInfo() const noexcept
		{
			return targetRect->GetTypeInfo();
		}
		void JEditorGeometryTool::RectView::SetMaterial(const JUserPtr<JMaterial>& mat)
		{
			for (int i = 0; i < 4; ++i)
				innerLine[i]->GetRenderItem()->SetMaterial(0, mat);
			for (int i = 0; i < 4; ++i)
				edgeLine[i]->GetRenderItem()->SetMaterial(0, mat);
			for (int i = 0; i < 4; ++i)
				outerLine[i]->GetRenderItem()->SetMaterial(0, mat);
		}
		bool JEditorGeometryTool::RectView::IsValid()const noexcept
		{
			return root != nullptr && targetRect != nullptr && targetRect->IsActivated() && targetRect->GetOwner()->IsSelected();
		}

		JEditorGeometryTool::~JEditorGeometryTool()
		{
			Clear();
		}
		J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE JEditorGeometryTool::GetToolType()const noexcept
		{
			return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::GEO_VIEW;
		}
		void JEditorGeometryTool::TryCreateGeoView(const std::vector<JUserPtr<JGameObject>>& idenVec, const JUserPtr<JGameObject>& parent)
		{
			for (const auto& data : idenVec)
			{
				auto litVec = data->GetComponents(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				for (const auto& litData : litVec)
				{
					J_LIGHT_TYPE litType = static_cast<JLight*>(litData.Get())->GetLightType();
					if (litType == J_LIGHT_TYPE::POINT)
						CreateSphereView(Core::ConnectChildUserPtr<JPointLight>(litData), parent);
					else if (litType == J_LIGHT_TYPE::SPOT)
						CreateSpotView(Core::ConnectChildUserPtr<JSpotLight>(litData), parent);
					else if (litType == J_LIGHT_TYPE::RECT)
						CreateRectView(Core::ConnectChildUserPtr<JRectLight>(litData), parent);
				}
			}
		}
		bool JEditorGeometryTool::CreateFrustumView(const JUserPtr<JCamera>& cam, const JUserPtr<JGameObject>& parent)
		{
			if (cam == nullptr || HasGeo(cam->GetGuid()))
				return false;

			return CreateGeoView(std::make_unique<FrustumView>(cam, parent), cam->GetGuid());
		}
		bool JEditorGeometryTool::CreateSphereView(const JUserPtr<JPointLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit == nullptr || HasGeo(lit->GetGuid()))
				return false;

			return CreateGeoView(std::make_unique<SphereView>(lit, parent), lit->GetGuid());
		}
		bool JEditorGeometryTool::CreateSpotView(const JUserPtr<JSpotLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit == nullptr || HasGeo(lit->GetGuid()))
				return false;

			return CreateGeoView(std::make_unique<ConeView>(lit, parent), lit->GetGuid());
		}
		bool JEditorGeometryTool::CreateRectView(const JUserPtr<JRectLight>& lit, const JUserPtr<JGameObject>& parent)
		{
			if (lit == nullptr || HasGeo(lit->GetGuid()))
				return false;

			return CreateGeoView(std::make_unique<RectView>(lit, parent), lit->GetGuid());
		}
		bool JEditorGeometryTool::CreateGeoView(std::unique_ptr<GeometryView>&& view, const size_t guid)
		{
			//HasGeo(guid)
			if (view == nullptr || !view->IsValid())
				return false;

			geoSet.emplace(guid);
			geoView.push_back(std::move(view));
			return true;
		}
		void JEditorGeometryTool::DestroyView(const size_t guid)
		{
			if (!HasGeo(guid))
				return;

			for (int i = 0; i < geoView.size(); ++i)
			{
				if (geoView[i]->GetTargetGuid() == guid)
				{
					geoSet.erase(geoView[i]->GetTargetGuid());
					geoView.erase(geoView.begin() + i);
					return;
				}
			}
		}
		void JEditorGeometryTool::Clear()
		{
			geoSet.clear();
			for (const auto& data : geoView)
				data->Clear();
			geoView.clear();
		}
		void JEditorGeometryTool::ClearTarget(Core::JTypeInfo& type)
		{
			for (int i = 0; i < geoView.size(); ++i)
			{
				if (geoView[i]->GetTargetTypeInfo().IsChildOf(type))
				{
					geoSet.erase(geoView[i]->GetTargetGuid());
					geoView.erase(geoView.begin() + i);
					--i;
				}
			}
		}
		void JEditorGeometryTool::Update()
		{
			std::vector<uint> invalidVec;
			const uint count = (uint)geoView.size();
			for (uint i = 0; i < count; ++i)
			{
				if (!geoView[i]->IsValid())
					invalidVec.push_back(i);
				else
					geoView[i]->Update();
			}

			const int invalidCount = (int)invalidVec.size();
			for (int i = invalidCount - 1; i >= 0; --i)
			{
				geoSet.erase(geoView[invalidVec[i]]->GetTargetGuid());
				geoView.erase(geoView.begin() + invalidVec[i]);
			}
		}
		bool JEditorGeometryTool::HasGeo(const size_t guid)
		{
			return geoSet.find(guid) != geoSet.end();
		}
	}
}