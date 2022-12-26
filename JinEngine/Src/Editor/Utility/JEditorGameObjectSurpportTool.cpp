#include"JEditorGameObjectSurpportTool.h"
#include"../Page/JEditorPageShareData.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Object/Component/Camera/JCamera.h"  
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectFactory.h"
#include"../../Object/GameObject/JGameObjectFactoryUtility.h"   
#include"../../Object/Resource/Material/JDefaultMaterialType.h" 
#include"../../Object/Resource/JResourceManager.h" 
#include"../../Object/Resource/JResourceObjectFactory.h" 

using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{
		void JEditorTransformTool::Arrow::CreateMaterial(const JVector4<float> matColor)
		{			
			JDirectory* dir = JResourceManager::Instance().GetEditorResourceDirectory();
			JMaterial* material = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"ArrowMaterial",
				Core::MakeGuid(),
				OBJECT_FLAG_EDITOR_OBJECT,
				dir));

			material->SetAlbedoColor(matColor.ConvertXMF());
			material->SetDebugMaterial(true);

			JEditorTransformTool::Arrow::material = Core::GetUserPtr(material);
		}
		void JEditorTransformTool::Arrow::Initialze(JGameObject* debugRoot, const J_DEFAULT_SHAPE shape,  const JVector3<float> initRotation)
		{
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT; 
			JGameObject* arrow = JGFU::CreateShape(*debugRoot, flag, shape); 
			arrow->GetTransform()->SetRotation(initRotation.ConvertXMF());
 
			JRenderItem* rItem = arrow->GetRenderItem();
			const uint subMeshCount = rItem->GetSubmeshCount();
			for (uint j = 0; j < subMeshCount; ++j)
				rItem->SetMaterial(j, material.Get());
			rItem->SetRenderLayer(J_RENDER_LAYER::DEBUG_LAYER);

			//arrow->GetTransform()->SetScale(initScale.ConvertXMF());

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
		bool JEditorTransformTool::Arrow::IsClick(Core::JUserPtr<JCamera> cam, bool onDebug)
		{
			const BoundingBox bbox = arrow->GetRenderItem()->GetBoundingBox(true);
			 
			const XMFLOAT3 minF = JMathHelper::Vector3Minus(bbox.Center, bbox.Extents);
			const XMFLOAT3 maxF = JMathHelper::Vector3Plus(bbox.Center, bbox.Extents);

			const XMMATRIX viewProj = XMMatrixMultiply(cam->GetView(), cam->GetProj());
			const XMVECTOR minPV = XMVector3Transform(XMLoadFloat3(&minF), viewProj);
			const XMVECTOR maxPV = XMVector3Transform(XMLoadFloat3(&maxF), viewProj);
			XMFLOAT4 minPF;
			XMStoreFloat4(&minPF, minPV);
			XMFLOAT4 maxPF;
			XMStoreFloat4(&maxPF, maxPV);

			const XMVECTOR minNDCV = XMVectorSet(minPF.x / minPF.w, minPF.y / minPF.w, minPF.z / minPF.w, 1) * 0.5f + XMVectorSet(0.5f, 0.5f, 1, 1);;
			const XMVECTOR maxNDCV = XMVectorSet(maxPF.x / maxPF.w, maxPF.y / maxPF.w, maxPF.z / maxPF.w, 1) * 0.5f + XMVectorSet(0.5f, 0.5f, 1, 1);;
			 
			XMFLOAT3 minNDCF;
			XMStoreFloat3(&minNDCF, minNDCV);
			XMFLOAT3 maxNDCF;
			XMStoreFloat3(&maxNDCF, maxNDCV);

			JVector2<uint> camViewSize = JVector2<uint>(cam->GetViewWidth(), cam->GetViewHeight());
			JVector2<uint> windowPos = ImGui::GetWindowPos();
			JVector2<uint> windowSize = ImGui::GetWindowSize();
			JVector2<float> sizeRate = JVector2<float>((float)windowSize.x / camViewSize.x, (float)windowSize.y / camViewSize.y );
 
			JVector2<float> minPoint = JVector2<float>(minNDCF.x * windowSize.x  + windowPos.x, minNDCF.y  * windowSize.y + windowPos.y);
			JVector2<float> maxPoint = JVector2<float>(maxNDCF.x * windowSize.x  + windowPos.x, maxNDCF.y  * windowSize.y + windowPos.y);
			
			if (onDebug)
			{
				ImGui::Text((std::to_string(minPoint.x) + " " + std::to_string(minPoint.y)).c_str()); 
				ImGui::SameLine();
				ImGui::Text((std::to_string(maxPoint.x) + " " + std::to_string(maxPoint.y)).c_str());
				ImGui::SameLine();
				ImGui::Text((std::to_string(ImGui::GetMousePos().x) + " " + std::to_string(ImGui::GetMousePos().y)).c_str());
			} 
			if (JImGuiImpl::IsMouseInRect(minPoint, maxPoint - minPoint))
				return true;
			else
				return false;
		}
		void JEditorTransformTool::Arrow::OnDraaing()
		{
			material->SetAlbedoColor(JMathHelper::Vector4Plus(material->GetAlbedoColor(), XMFLOAT4(0.5f, 0.5f, 0.5f, 0)));
		}
		void JEditorTransformTool::Arrow::OffDragging()
		{
			material->SetAlbedoColor(JMathHelper::Vector4Minus(material->GetAlbedoColor(), XMFLOAT4(0.5f, 0.5f, 0.5f, 0)));
		}
		JEditorTransformTool::JEditorTransformTool(const J_DEFAULT_SHAPE shape, const float sizeRate)
			:shape(shape), sizeRate(sizeRate)
		{
			const XMFLOAT3 extents = JResourceManager::Instance().GetDefaultMeshGeometry(shape)->GetBoundingBox().Extents;
			shapeLength = JMathHelper::Vector3Length(extents) * 2;

			JVector4<float> color[Constants::arrowCount]
			{
				JVector4<float>{0.8f, 0, 0, 0.75f},
				JVector4<float>{0, 0.8f, 0, 0.75f},
				JVector4<float>{0, 0, 0.8f, 0.75f},
			};

			for (uint i = 0; i < Constants::arrowCount; ++i)
				arrow[i].CreateMaterial(color[i]);
		}
		void JEditorTransformTool::Activate()
		{
			J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
			JGameObject* transformArrowRoot = JGFI::Create(L"Transform Arrow Root", Core::MakeGuid(), flag, *debugRoot);
			//x y z
			JVector3<float> rot[3]
			{
				JVector3<float>{0, 0, -90},
				JVector3<float>{0, 0, 0},
				JVector3<float>{90, 0, 0},
			};

			for (uint i = 0; i < 3; ++i)
				arrow[i].Initialze(transformArrowRoot, shape, rot[i]);

			JEditorTransformTool::transformArrowRoot = Core::GetUserPtr(transformArrowRoot);
 
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
			if (!transformArrowRoot.IsValid() || !selected.IsValid() || !cam.IsValid() || 
				selected->GetObjectType() != J_OBJECT_TYPE::GAME_OBJECT)
			{
				if (isActivated)
					DeActivate();
				return;
			} 

			JGameObject* gameObject = static_cast<JGameObject*>(selected.Get());
			if (!gameObject->HasRenderItem())
			{
				if (isActivated)
					DeActivate();
				return;
			}

			if (!isActivated)
				Activate();

			UpdateSelectedTransform(gameObject);
			UpdateArrowPosition(gameObject, cam);
			//UpdateArrowDragging(gameObject, cam);
		}
		void JEditorTransformTool::UpdateSelectedTransform(JGameObject* selected)
		{
			if (!ImGui::IsMouseDragging(0))
				return;

			ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0);
		}
		void JEditorTransformTool::UpdateArrowPosition(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{ 
			const JVector2<uint> wndsize = ImGui::GetWindowSize();
			transformArrowRoot->GetTransform()->SetPosition(selected->GetRenderItem()->GetBoundingBox().Center);

			const XMFLOAT3 centerF = selected->GetRenderItem()->GetBoundingBox().Center;
			const XMFLOAT3 etxtentF = JMathHelper::Vector3Plus(centerF, arrow[0].arrow->GetRenderItem()->GetMesh()->GetBoundingBox().Extents);
			const XMMATRIX viewProj = XMMatrixMultiply(cam->GetView(), cam->GetProj());
			const XMVECTOR centerPV = XMVector3Transform(XMLoadFloat3(&centerF), viewProj);
			const XMVECTOR extentPV = XMVector3Transform(XMLoadFloat3(&etxtentF), viewProj);
			XMFLOAT4 centerPF;
			XMStoreFloat4(&centerPF, centerPV);
			XMFLOAT4 extentPF;
			XMStoreFloat4(&extentPF, extentPV);

			const XMVECTOR centerNDCV = XMVectorSet(centerPF.x / centerPF.w, centerPF.y / centerPF.w, centerPF.z / centerPF.w, 1) * 0.5f + XMVectorSet(0.5f, -0.5f, 1, 1);;
			const XMVECTOR extentNDCV = XMVectorSet(extentPF.x / extentPF.w, extentPF.y / extentPF.w, extentPF.z / extentPF.w, 1) * 0.5f + XMVectorSet(0.5f, -0.5f, 1, 1);;

			const XMVECTOR distanceNDCV = XMVectorSubtract(extentNDCV, centerNDCV);
			XMFLOAT3 distanceNDCF;
			XMStoreFloat3(&distanceNDCF, distanceNDCV);

			float maxDis = distanceNDCF.x > distanceNDCF.y ? distanceNDCF.x : distanceNDCF.y;
			float scaleFactor = sizeRate / maxDis;
			if (maxDis >= sizeRate)
				transformArrowRoot->GetTransform()->SetScale(XMFLOAT3(1, 1, 1));
			else
				transformArrowRoot->GetTransform()->SetScale(XMFLOAT3(scaleFactor, scaleFactor, scaleFactor));
		}
		void JEditorTransformTool::UpdateArrowDragging(JGameObject* selected, Core::JUserPtr<JCamera> cam)
		{
			if (!isDraggingObject)
			{
				if (!ImGui::IsMouseDown(0))
					return;

				int clickedIndex = -1;
				for (uint i = 0; i < Constants::arrowCount; ++i)
				{ 
					if (arrow[i].IsClick(cam, i == 0))
					{
						clickedIndex = i;
						break;
					}
				}
				if (clickedIndex == -1)
					return;
				 
				arrow[clickedIndex].OnDraaing();
				isDraggingObject = true;
				draggingIndex = clickedIndex;
			}
			else
			{
				if (!ImGui::IsMouseDragging(0))
				{
					arrow[draggingIndex].OffDragging();
					isDraggingObject = false;
					draggingIndex = -1;
				}
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