#include"JObjectDetail.h"
#include"../../JEditorAttribute.h" 
#include"../../../Utility/JEditorWidgetPosCalculator.h" 
#include"../../../String/JEditorString.h"
#include"../../../../Utility/JMathHelper.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../Core/Reflection/JReflectionInfo.h"

#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../../Object/Resource/Material/JMaterial.h" 
#include"../../../../Object/Resource/Texture/JTexture.h" 
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Model/JModel.h"
#include"../../../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../../../../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../../../../Object/Resource/Shader/JShader.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"

#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Animator/JAnimator.h"
#include"../../../../Object/Component/Light/JLight.h" 
#include"../../../../Object/Component/JComponentFactoryUtility.h" 

#include"../../../../../Lib/imgui/imgui.h"  

using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{
		JObjectDetail::JObjectDetail(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid), previewGuid(JCommonUtility::CalculateGuid(GetName() + "PreviewGroup"))
		{
			editorString = std::make_unique<JEditorString>();

			animatiorDetail.animatorBtnId = JCommonUtility::CalculateGuid("Select JAnimator Btn");
			animatiorDetail.animatorWindowId = JCommonUtility::CalculateGuid("Select JAnimator Window");
			animatiorDetail.skeletonBtnId = JCommonUtility::CalculateGuid("Select JSkeleton Btn");
			animatiorDetail.skeletonWindowId = JCommonUtility::CalculateGuid("Select JSkeleton Window");

			editorString->AddString(animatiorDetail.animatorBtnId, { "Select JAnimator: ", u8"애니메이터 선택: " });
			editorString->AddString(animatiorDetail.animatorWindowId, { "Select JAnimator Window", u8"애니메이터 선택창" });
			editorString->AddString(animatiorDetail.skeletonBtnId, { "Select JSkeletonAsset: ", u8"스켈레톤애셋 선택: " });
			editorString->AddString(animatiorDetail.skeletonWindowId, { "Select JSkeletonAsset Window", u8"스켈레톤애셋 선택창" });

			componentSelectorDetail.componentBtnId = JCommonUtility::CalculateGuid("Add JComponent");
			componentSelectorDetail.componentListboxId = JCommonUtility::CalculateGuid("JComponent List");

			editorString->AddString(componentSelectorDetail.componentBtnId, { "Add JComponent", u8"컴포넌트 추가" });
			editorString->AddString(componentSelectorDetail.componentListboxId, { "JComponent List", u8"컴포넌트 리스트" });

			meshDetail.meshBtnId = JCommonUtility::CalculateGuid("Select Mesh");
			meshDetail.meshWindowId = JCommonUtility::CalculateGuid("Select Mesh Window");

			editorString->AddString(meshDetail.meshBtnId, { "Select Mesh: ", u8"메시 선택: " });
			editorString->AddString(meshDetail.meshWindowId, { "Select Mesh Window", u8"메시 선택창" });

			materialDetail.materialBtnId = JCommonUtility::CalculateGuid("Select JMaterial");
			materialDetail.materialWindowId = JCommonUtility::CalculateGuid("Select JMaterial Window");
			materialDetail.textureWindowId = JCommonUtility::CalculateGuid("Select JTexture Window");

			editorString->AddString(materialDetail.materialBtnId, { "Select JMaterial: ", u8"머테리얼 선택: " });
			editorString->AddString(materialDetail.materialWindowId, { "Select JMaterial Window", u8"머테리얼 선택창" });
			editorString->AddString(materialDetail.textureWindowId, { "Select JMaterial Window", u8"텍스쳐 선택창" });

			selectorIconSlidebarId = JCommonUtility::CalculateGuid("Resource Selector Icon Slidebar");
			editorString->AddString(selectorIconSlidebarId, { "Size", u8"크기조절" });

			editorPositionCal = std::make_unique<JEditorWidgetPosCalculator>();
			previewGroup = JResourceManager::Instance().CreatePreviewGroup(GetName(), previewCapacity);
		}
		void JObjectDetail::Initialize(JEditorUtility* editorUtility)noexcept
		{
		}
		void JObjectDetail::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);
			SearchSelectorEvVec();

			windowWidth = ImGui::GetWindowWidth();
			windowHeight = editorUtility->displayHeight * GetInitHeightRate();
			selectorIconMaxSize = editorUtility->displayWidth * selectorIconMaxRate;
			selectorIconMinSize = editorUtility->displayWidth * selectorIconMinRate;
			selectorIconSize = JMathHelper::Clamp<float>(selectorIconSize, selectorIconMinSize, selectorIconMaxSize);

			float oriSize = ImGui::GetStyle().ChildBorderSize;
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);

			SelectedObjectDetailOnScreen(editorUtility);

			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, oriSize);
		}
		bool JObjectDetail::Activate(JEditorUtility* editorUtility)
		{
			if (JEditor::Activate(editorUtility))
			{
				std::vector<J_EDITOR_EVENT> enumVec
				{
					J_EDITOR_EVENT::MOUSE_CLICK,J_EDITOR_EVENT::SELECT_RESOURCE,J_EDITOR_EVENT::SELECT_GAMEOBJECT, J_EDITOR_EVENT::DESELECT_GAMEOBJECT
				};
				this->AddEventListener(*editorUtility->EvInterface(), GetGuid(), enumVec);
				return true;
			}
			else
				return false;
		}
		bool JObjectDetail::DeActivate(JEditorUtility* editorUtility)
		{
			if (JEditor::DeActivate(editorUtility))
			{
				this->RemoveListener(*editorUtility->EvInterface(), GetGuid());
				JResourceManager::Instance().ClearPreviewGroup(previewGroup);
				selectorWindowKey = false;
				return true;
			}
			else
				return false;
		}
		bool JObjectDetail::OnFocus(JEditorUtility* editorUtility)
		{
			if (JEditor::OnFocus(editorUtility))
				return true;
			else
				return false;
		}
		bool JObjectDetail::OffFocus(JEditorUtility* editorUtility)
		{
			if (JEditor::OffFocus(editorUtility))
			{
				selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				return true;
			}
			else
				return false;
		}
		void JObjectDetail::SelectedObjectDetailOnScreen(JEditorUtility* editorUtility)
		{
			//MessageBox(0, std::to_wstring(nowSize.x).c_str(), std::to_wstring(nowPos.x).c_str(), 0);
			if (editorUtility->selectedObject != nullptr)
			{
				J_OBJECT_TYPE objType = editorUtility->selectedObject->GetObjectType();
				switch (objType)
				{
				case J_OBJECT_TYPE::GAME_OBJECT:
				{
					JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
					TransformDetailOnScreen(editorUtility, gameObj);
					if (gameObj->HasAnimator())
						AnimatorDetailOnScreen(editorUtility, gameObj->GetAnimator());
					if (gameObj->HasRenderItem())
					{
						MeshDetailOnScreen(editorUtility, gameObj->GetRenderItem()->GetMesh(), J_OBJECT_TYPE::GAME_OBJECT);
						MaterialDetailOnScreen(editorUtility, gameObj->GetRenderItem()->GetMaterial(), J_OBJECT_TYPE::GAME_OBJECT);
					}
					JCamera* camera = gameObj->GetComponent<JCamera>();
					if (camera != nullptr)
						CameraDetailOnScreen(editorUtility, camera);

					std::vector<JLight*> light = gameObj->GetComponents<JLight>();
					const uint lightCount = (uint)light.size();
					for (uint i = 0; i < lightCount; ++i)
						LightDetailOnScreen(editorUtility, light[i]);

					ShowComponentSelector(editorUtility, gameObj);
					break;
				}
				case J_OBJECT_TYPE::RESOURCE_OBJECT:
				{
					JResourceObject* resourceObj = dynamic_cast<JResourceObject*>(editorUtility->selectedObject);
					const J_RESOURCE_TYPE resourceType = resourceObj->GetResourceType();
					if (resourceType == J_RESOURCE_TYPE::ANIMATION_CLIP)
						AnimationClipDataOnScreen(editorUtility, static_cast<JAnimationClip*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					else if (resourceType == J_RESOURCE_TYPE::MESH)
						MeshDetailOnScreen(editorUtility, static_cast<JMeshGeometry*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					else if (resourceType == J_RESOURCE_TYPE::MATERIAL)
						MaterialDetailOnScreen(editorUtility, static_cast<JMaterial*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					else if (resourceType == J_RESOURCE_TYPE::MODEL)
						ModelDataOnScreen(editorUtility, static_cast<JModel*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					else if (resourceType == J_RESOURCE_TYPE::TEXTURE)
						TextureDataOnScreen(editorUtility, static_cast<JTexture*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					else if (resourceType == J_RESOURCE_TYPE::SKELETON)
						SkeletonAssetDataOnScreen(editorUtility, static_cast<JSkeletonAsset*>(resourceObj), J_OBJECT_TYPE::GAME_OBJECT);
					break;
				}
				default:
					break;
				}
			}
		}
		void JObjectDetail::TransformDetailOnScreen(JEditorUtility* editorUtility, JGameObject* gameObj)
		{
			if (gameObj != nullptr)
			{
				JTransform* transform = gameObj->GetTransform();
				XMFLOAT3 oldPosition = transform->GetPosition();
				XMFLOAT3 oldRotation = transform->GetRotation();
				XMFLOAT3 oldScale = transform->GetScale();

				transformDetail.positionBuff[0] = oldPosition.x;
				transformDetail.positionBuff[1] = oldPosition.y;
				transformDetail.positionBuff[2] = oldPosition.z;

				transformDetail.rotationBuff[0] = oldRotation.x;
				transformDetail.rotationBuff[1] = oldRotation.y;
				transformDetail.rotationBuff[2] = oldRotation.z;

				transformDetail.scaleBuff[0] = oldScale.x;
				transformDetail.scaleBuff[1] = oldScale.y;
				transformDetail.scaleBuff[2] = oldScale.z;

				ImGui::BeginChild("JTransform", ImVec2(windowWidth, windowHeight * 0.15f), true, ImGuiWindowFlags_NoDocking);

				ImGuiTableFlags flag = ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_BordersV |
					ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;

				ImGui::Text("JTransform");
				if (ImGui::BeginTable("##TransformTable", 4, flag))
				{
					ImGui::TableSetupColumn("##JTransform Guide Text", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("X##JTransform", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Y##JTransform", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Z##JTransform", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();

					for (int row = 0; row < 3; row++)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(transformDetail.guideText[row].c_str());
						for (int column = 1; column < 4; column++)
						{
							ImGui::TableSetColumnIndex(column);
							ImGui::InputFloat(("##TransformValue" + std::to_string(row) + std::to_string(column)).c_str(), transformDetail.valueBuffer[row] + (column - 1));
						}
					}
					ImGui::EndTable();
				}

				XMFLOAT3 newPosition(transformDetail.positionBuff[0], transformDetail.positionBuff[1], transformDetail.positionBuff[2]);
				XMFLOAT3 newRotation(transformDetail.rotationBuff[0], transformDetail.rotationBuff[1], transformDetail.rotationBuff[2]);
				XMFLOAT3 newScale(transformDetail.scaleBuff[0], transformDetail.scaleBuff[1], transformDetail.scaleBuff[2]);

				if (newPosition.x != oldPosition.x ||
					newPosition.y != oldPosition.y ||
					newPosition.z != oldPosition.z)
					transform->SetPosition(newPosition);
				if (newRotation.x != oldRotation.x ||
					newRotation.y != oldRotation.y ||
					newRotation.z != oldRotation.z)
					transform->SetRotation(newRotation);
				if (newScale.x != oldScale.x ||
					newScale.y != oldScale.y ||
					newScale.z != oldScale.z)
					transform->SetScale(newScale);

				ImGui::EndChild();
			}
		}
		void JObjectDetail::AnimatorDetailOnScreen(JEditorUtility* editorUtility, JAnimator* animator)
		{
			ImGui::BeginChild("JAnimator", ImVec2(windowWidth, windowHeight * 0.15f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JAnimator");

			JAnimationController* aniController = animator->GetAnimatorController();
			if (aniController != nullptr)
				ImGui::Text(aniController->GetName().c_str());
			if (ImGui::Button(editorString->GetString(animatiorDetail.animatorBtnId).c_str(), ImVec2(windowWidth, 0)))
			{
				if (nowSelectorState != SELECTOR_STATE::OPEN_ANIMATION_CONTROLLER)
					selectorEvVec.push_back(SELECTOR_STATE::OPEN_ANIMATION_CONTROLLER);
				else
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			}
			if (nowSelectorState == SELECTOR_STATE::OPEN_ANIMATION_CONTROLLER)
			{
				JResourceObject* res = nullptr;
				if (ResourceSelectorOnScreen(editorString->GetString(animatiorDetail.animatorWindowId).c_str(), editorUtility, &res))
				{
					animator->SetAnimatorController(static_cast<JAnimationController*>(res));
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
			}

			JSkeletonAsset* skeletonAasset = animator->GetSkeletonAsset();
			if (skeletonAasset != nullptr)
				ImGui::Text(skeletonAasset->GetName().c_str());

			if (ImGui::Button(editorString->GetString(animatiorDetail.skeletonBtnId).c_str(), ImVec2(windowWidth, 0)))
			{
				if (nowSelectorState != SELECTOR_STATE::OPEN_SKELETON)
					selectorEvVec.push_back(SELECTOR_STATE::OPEN_SKELETON);
				else
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			}
			if (nowSelectorState == SELECTOR_STATE::OPEN_SKELETON)
			{
				JResourceObject* res = nullptr;
				if (ResourceSelectorOnScreen(editorString->GetString(animatiorDetail.skeletonWindowId).c_str(), editorUtility, &res))
				{
					animator->SetSkeletonAsset(dynamic_cast<JSkeletonAsset*>(res));
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
			}

			ImGui::EndChild();
		}
		void JObjectDetail::CameraDetailOnScreen(JEditorUtility* editorUtility, JCamera* camera)
		{
			ImGui::BeginChild("JCamera", ImVec2(windowWidth, windowHeight * 0.15f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JCamera");

			bool isMainCmaera = camera->IsMainCamera();
			float aspect = camera->GetAspect();
			float fov = camera->GetFovY();

			if (ImGui::Checkbox("Main JCamera##CheckBox", &isMainCmaera))
			{
				if (isMainCmaera)
					camera->SetMainCamera();
			}
			ImGui::Text(("Aspect: " + std::to_string(aspect)).c_str());
			ImGui::Text(("Fov: " + std::to_string(fov)).c_str());
			ImGui::EndChild();
		}
		void JObjectDetail::LightDetailOnScreen(JEditorUtility* editorUtility, JLight* light)
		{
			ImGui::BeginChild("JLight", ImVec2(windowWidth, windowHeight * 0.15f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JLight");

			bool shadowMapBtn = light->IsShadowActivated();
			if (ImGui::Checkbox("On JShadow##CheckBox", &shadowMapBtn))
				light->SetShadow(shadowMapBtn);
			ImGui::EndChild();
		}
		void JObjectDetail::ShowComponentSelector(JEditorUtility* editorUtility, JGameObject* gameObj)
		{
			if (gameObj != nullptr)
			{
				if (ImGui::Button(editorString->GetString(componentSelectorDetail.componentBtnId).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_COMPONENT)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_COMPONENT);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}

				if (nowSelectorState == SELECTOR_STATE::OPEN_COMPONENT)
				{
					std::vector<Core::JTypeInfo*>component =  Core::JReflectionInfo::Instance().GetDerivedTypeInfo(JComponent::StaticTypeInfo());

					const uint componentCount = (uint)component.size();
					int selectIndex = -1;

					ImGui::Text(editorString->GetString(componentSelectorDetail.componentListboxId).c_str());
					ImGui::BeginListBox("##ComponentSelector List");
					for (uint i = 0; i < componentCount; ++i)
					{
						if (ImGui::Selectable(component[i]->Name().c_str()))
							selectIndex = i;
					}
					ImGui::EndListBox();

					if (selectIndex != -1)
					{ 
						JComponentFactoryUtility::CreateComponent(component[selectIndex]->Name(), *gameObj);
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
					}
				}
			}
		}
		void JObjectDetail::MeshDetailOnScreen(JEditorUtility* editorUtility, JMeshGeometry* mesh, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("Mesh", ImVec2(windowWidth, windowHeight * 0.2f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("Mesh");
			if (objType == J_OBJECT_TYPE::GAME_OBJECT)
			{
				if (ImGui::Button(editorString->GetString(meshDetail.meshBtnId).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_MESH)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_MESH);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
			}
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			if (mesh != nullptr)
			{
				if (mesh->GetMeshGeometryType() == J_MESHGEOMETRY_TYPE::SKINNED)
					ImGui::Text("SkinnedMesh");
				else
					ImGui::Text("StaticMesh");
				ImGui::Text(("MeshVertex: " + std::to_string(mesh->GetMeshVertexCount())).c_str());
				ImGui::Text(("Index: " + std::to_string(mesh->GetMeshIndexCount())).c_str());
			}
			ImGui::EndChild();

			if (nowSelectorState == SELECTOR_STATE::OPEN_MESH)
			{
				JResourceObject* res = nullptr;
				if (ResourceSelectorOnScreen(editorString->GetString(meshDetail.meshWindowId).c_str(), editorUtility, &res))
				{
					if (res != nullptr)
					{
						JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
						JRenderItem* renderItem = gameObj->GetRenderItem();
						if (renderItem != nullptr)
							renderItem->SetMeshGeometry(dynamic_cast<JMeshGeometry*>(res));
					}
					else if (res == nullptr)
					{
						JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
						JRenderItem* renderItem = gameObj->GetRenderItem();
						if (renderItem != nullptr)
							renderItem->SetMeshGeometry(nullptr);
					}
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
			}
		}
		void JObjectDetail::MaterialDetailOnScreen(JEditorUtility* editorUtility, JMaterial* material, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("JMaterial", ImVec2(windowWidth, windowHeight * 0.25f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JMaterial");
			if (ImGui::Button(editorString->GetString(materialDetail.materialBtnId).c_str(), ImVec2(windowWidth, 0)))
			{
				if (nowSelectorState != SELECTOR_STATE::OPEN_MATERIAL)
					selectorEvVec.push_back(SELECTOR_STATE::OPEN_MATERIAL);
				else
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			}
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			if (material != nullptr)
			{
				JShader* nowShader = material->GetShader();
				J_SHADER_FUNCTION shaderFunc = nowShader->GetShaderFunctionFlag();

				if ((int)(JinEngine::SHADER_FUNCTION_ALBEDO_MAP & shaderFunc) > 0)
					ImGui::Text("Albedo map");
				if ((int)(JinEngine::SHADER_FUNCTION_ALBEDO_MAP_ONLY & shaderFunc) > 0)
					ImGui::Text("Albedo map only");
				if ((int)(JinEngine::SHADER_FUNCTION_NORMAL_MAP & shaderFunc) > 0)
					ImGui::Text("Normal map");
				if ((int)(JinEngine::SHADER_FUNCTION_HEIGHT_MAP & shaderFunc) > 0)
					ImGui::Text("Height map");
				if ((int)(JinEngine::SHADER_FUNCTION_ROUGHNESS_MAP & shaderFunc) > 0)
					ImGui::Text("Roughness map");
				if ((int)(JinEngine::SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP & shaderFunc) > 0)
					ImGui::Text("Ambient map");
				if ((int)(JinEngine::SHADER_FUNCTION_SHADOW & shaderFunc) > 0)
					ImGui::Text("JShadow");
				if ((int)(JinEngine::SHADER_FUNCTION_LIGHT & shaderFunc) > 0)
					ImGui::Text("JLight");
				if ((int)(JinEngine::SHADER_FUNCTION_SKY & shaderFunc) > 0)
					ImGui::Text("Sky map");
				if ((int)(JinEngine::SHADER_FUNCTION_NONCULLING & shaderFunc) > 0)
					ImGui::Text("non culling");
				if ((int)(JinEngine::SHADER_FUNCTION_ALPHA_CLIP & shaderFunc) > 0)
					ImGui::Text("Alpha clip");
				if ((int)(JinEngine::SHADER_FUNCTION_SHADOW_MAP & shaderFunc) > 0)
					ImGui::Text("JShadow map");

				J_OBJECT_FLAG objFlag = material->GetFlag();
				bool editable = ((int)objFlag & OBJECT_FLAG_UNEDITABLE) == 0;

				JVector4<float> oldAlbedoColor = material->GetAlbedoColor();

				DirectX::XMFLOAT4X4 oldMatTransform = material->GetMatTransform();
				float oldMetallc = material->GetMetallic();
				float oldRoughness = material->GetRoughness();

				materialDetail.albedoColorBuff[0] = oldAlbedoColor.x;
				materialDetail.albedoColorBuff[1] = oldAlbedoColor.y;
				materialDetail.albedoColorBuff[2] = oldAlbedoColor.z;
				materialDetail.albedoColorBuff[3] = oldAlbedoColor.a;

				materialDetail.materialTransformVectorXBuff[0] = oldMatTransform._11;
				materialDetail.materialTransformVectorXBuff[1] = oldMatTransform._12;
				materialDetail.materialTransformVectorXBuff[2] = oldMatTransform._13;

				materialDetail.materialTransformVectorYBuff[0] = oldMatTransform._21;
				materialDetail.materialTransformVectorYBuff[1] = oldMatTransform._22;
				materialDetail.materialTransformVectorYBuff[2] = oldMatTransform._23;

				materialDetail.materialTransformVectorZBuff[0] = oldMatTransform._31;
				materialDetail.materialTransformVectorZBuff[1] = oldMatTransform._32;
				materialDetail.materialTransformVectorZBuff[2] = oldMatTransform._33;

				materialDetail.metalicBuff = oldMetallc;
				materialDetail.roughnessBuff = oldRoughness;

				ImGui::Text("Albedo");
				ImGui::SliderFloat("R", &materialDetail.albedoColorBuff[0], 0, 1);
				ImGui::SliderFloat("G", &materialDetail.albedoColorBuff[1], 0, 1);
				ImGui::SliderFloat("B", &materialDetail.albedoColorBuff[2], 0, 1);
				ImGui::SliderFloat("A", &materialDetail.albedoColorBuff[3], 0, 1);
				std::string albedoMapName = "None";
				std::string normalMapName = "None";
				std::string heightName = "None";
				std::string roughnessName = "None";
				std::string ambientName = "None";

				if (material->HasAlbedoMapTexture())
					albedoMapName = material->GetDiffsueMap()->GetName();
				if (material->HasNormalMapTexture())
					normalMapName = material->GetNormalMap()->GetName();
				if (material->HasHeightMapTexture())
					heightName = material->GetHeightMap()->GetName();
				if (material->HasRoughnessMapTexture())
					roughnessName = material->GetRoughnessMap()->GetName();
				if (material->HasAmbientOcclusionMapTexture())
					ambientName = material->GetAmbientOcclusionMap()->GetName();

				if (editable && ImGui::Button(("AlbedoMap: " + albedoMapName).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_TEXTURE_ALBEDO)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_TEXTURE_ALBEDO);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
				if (editable && ImGui::Button(("NormalMap: " + normalMapName).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_TEXTURE_NORMAL)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_TEXTURE_NORMAL);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
				if (editable && ImGui::Button(("HeightMap: " + heightName).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_TEXTURE_HEIGHT)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_TEXTURE_HEIGHT);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
				if (editable && ImGui::Button(("RoughnessMap: " + roughnessName).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_TEXTURE_ROUGHNESS)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_TEXTURE_ROUGHNESS);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
				if (editable && ImGui::Button(("AmbientOcclusionMap: " + ambientName).c_str(), ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_TEXTURE_AMBIENT)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_TEXTURE_AMBIENT);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::Text("Roughness");
				ImGui::SliderFloat("##Roughness", &materialDetail.roughnessBuff, 0, 1);

				ImGui::Text("Metallic");
				ImGui::SliderFloat("##Metallic", &materialDetail.metalicBuff, 0, 1);

				ImGui::Text("JTransform");
				DirectX::XMFLOAT4X4 matrix = material->GetMatTransform();

				ImGui::Text((std::to_string(matrix._11) + " " +
					std::to_string(matrix._12) + " " +
					std::to_string(matrix._13) + " " +
					std::to_string(matrix._14)).c_str());
				ImGui::Text((std::to_string(matrix._21) + " " +
					std::to_string(matrix._22) + " " +
					std::to_string(matrix._23) + " " +
					std::to_string(matrix._24)).c_str());
				ImGui::Text((std::to_string(matrix._31) + " " +
					std::to_string(matrix._32) + " " +
					std::to_string(matrix._33) + " " +
					std::to_string(matrix._34)).c_str());
				ImGui::Text((std::to_string(matrix._41) + " " +
					std::to_string(matrix._42) + " " +
					std::to_string(matrix._43) + " " +
					std::to_string(matrix._44)).c_str());

				JVector4 newAlbedoColor(materialDetail.albedoColorBuff[0],
					materialDetail.albedoColorBuff[1],
					materialDetail.albedoColorBuff[2],
					materialDetail.albedoColorBuff[3]);

				float newMetallic = materialDetail.metalicBuff;
				float newRoughness = materialDetail.roughnessBuff;

				if (editable && newAlbedoColor != oldAlbedoColor)
					material->SetAlbedoColor(XMFLOAT4(newAlbedoColor.x, newAlbedoColor.y, newAlbedoColor.z, newAlbedoColor.a));

				if (editable && newMetallic != oldMetallc)
					material->SetMetallic(newMetallic);

				if (editable && newRoughness != oldRoughness)
					material->SetRoughness(newRoughness);

				if (editable)
				{
					switch (nowSelectorState)
					{
					case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_ALBEDO:
					{
						JResourceObject* res = nullptr;
						if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.textureWindowId).c_str(), editorUtility, &res))
						{
							if (res != nullptr)
								material->SetAlbedoMap(dynamic_cast<JTexture*>(res));
							else
								material->SetAlbedoMap(nullptr);
							selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
						}
					}
					break;
					case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_NORMAL:
					{
						JResourceObject* res = nullptr;
						if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.textureWindowId).c_str(), editorUtility, &res))
						{
							if (res != nullptr)
								material->SetNormalMap(dynamic_cast<JTexture*>(res));
							else
								material->SetNormalMap(nullptr);
							selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
						}
					}
					break;
					case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_HEIGHT:
					{
						JResourceObject* res = nullptr;
						if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.textureWindowId).c_str(), editorUtility, &res))
						{
							if (res != nullptr)
								material->SetHeightMap(dynamic_cast<JTexture*>(res));
							else
								material->SetHeightMap(nullptr);
							selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
						}
					}
					break;
					case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_ROUGHNESS:
					{
						JResourceObject* res = nullptr;
						if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.textureWindowId).c_str(), editorUtility, &res))
						{
							if (res != nullptr)
								material->SetRoughnessMap(dynamic_cast<JTexture*>(res));
							else
								material->SetRoughnessMap(nullptr);
							selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
						}
					}
					break;
					case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_AMBIENT:
					{
						JResourceObject* res = nullptr;
						if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.textureWindowId).c_str(), editorUtility, &res))
						{
							if (res != nullptr)
								material->SetAmbientOcclusionMap(dynamic_cast<JTexture*>(res));
							else
								material->SetAmbientOcclusionMap(nullptr);
							selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
						}
					}
					break;
					default:
						break;
					}
				}
			}
			ImGui::EndChild();

			if (nowSelectorState == SELECTOR_STATE::OPEN_MATERIAL)
			{
				JResourceObject* res = nullptr;
				if (ResourceSelectorOnScreen(editorString->GetString(materialDetail.materialBtnId).c_str(), editorUtility, &res))
				{
					if (res != nullptr)
					{
						JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
						JRenderItem* renderItem = gameObj->GetRenderItem();
						if (renderItem != nullptr)
							renderItem->SetMaterial(dynamic_cast<JMaterial*>(res));
					}
					else if (res == nullptr)
					{
						JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
						JRenderItem* renderItem = gameObj->GetRenderItem();
						if (renderItem != nullptr)
							renderItem->SetMaterial(nullptr);
					}
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
			}
		}
		void JObjectDetail::ModelDataOnScreen(JEditorUtility* editorUtility, JModel* model, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("JModel", ImVec2(windowWidth, windowHeight * 0.25f), true, ImGuiWindowFlags_NoDocking);
			if (model != nullptr)
			{
				ImGui::Text(("Name: " + model->GetName()).c_str());
				ImGui::Text(("Total Mesh: " + std::to_string(model->GetTotalMeshCount())).c_str());
				ImGui::Text(("Total JStaticMeshVertex: " + std::to_string(model->GetTotalVertexCount())).c_str());
				ImGui::Text(("Total Index: " + std::to_string(model->GetTotalIndexCount())).c_str());
			}
			ImGui::EndChild();
		}
		void JObjectDetail::AnimationClipDataOnScreen(JEditorUtility* editorUtility, JAnimationClip* animationClip, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("JAnimationClip", ImVec2(windowWidth, windowHeight * 0.25f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JAnimationClip");

			if (animationClip != nullptr)
			{
				JSkeletonAsset* skeletonAasset = animationClip->GetClipSkeletonAsset();
				if (skeletonAasset != nullptr)
					ImGui::Text(skeletonAasset->GetName().c_str());

				if (ImGui::Button("Select JSkeletonAsset: ", ImVec2(windowWidth, 0)))
				{
					if (nowSelectorState != SELECTOR_STATE::OPEN_SKELETON)
						selectorEvVec.push_back(SELECTOR_STATE::OPEN_SKELETON);
					else
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
				}
				if (nowSelectorState == SELECTOR_STATE::OPEN_SKELETON)
				{
					JResourceObject* res = nullptr;
					if (ResourceSelectorOnScreen("JSkeletonAsset Selector", editorUtility, &res))
					{
						animationClip->SetClipSkeletonAsset(dynamic_cast<JSkeletonAsset*>(res));
						selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
					}
				}
			}
			ImGui::EndChild();
		}
		void JObjectDetail::TextureDataOnScreen(JEditorUtility* editorUtility, JTexture* texture, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("JTexture", ImVec2(windowWidth, windowHeight * 0.25f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JTexture");

			if (texture != nullptr)
			{
				ImGui::Text(texture->GetName().c_str());
				ImGui::Text(std::to_string(texture->GetTextureWidth()).c_str());
				ImGui::Text(std::to_string(texture->GetTextureHeight()).c_str());
				Graphic::J_GRAPHIC_TEXTURE_TYPE textureType = texture->GetTextureType();
				switch (textureType)
				{
				case Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D:
					ImGui::Text("Texture2D");
					break;
				case Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE:
					ImGui::Text("TextureCubeMap");
					break;
				default:
					ImGui::Text("Error JTexture");
					break;
				}

			}
			ImGui::EndChild();
		}
		void JObjectDetail::SkeletonAssetDataOnScreen(JEditorUtility* editorUtility, JSkeletonAsset* skeletonAsset, const J_OBJECT_TYPE objType)
		{
			ImGui::BeginChild("JSkeletonAsset", ImVec2(windowWidth, windowHeight * 0.25f), true, ImGuiWindowFlags_NoDocking);
			ImGui::Text("JSkeletonAsset");

			if (skeletonAsset != nullptr)
				ImGui::Text(skeletonAsset->GetName().c_str());
			ImGui::EndChild();
		}
		bool JObjectDetail::ResourceSelectorOnScreen(const std::string& windowName, _In_ JEditorUtility* editorUtility, _Out_ JResourceObject** select)
		{
			ImGui::SetNextWindowSize(ImVec2(editorUtility->displayWidth * selectorWindowWidthRate, editorUtility->displayHeight * selectorWindowHeightRate));
			ImGui::Begin(windowName.c_str(), &selectorWindowKey, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text(editorString->GetString(selectorIconSlidebarId).c_str());
			ImGui::SameLine();
			int sliderOffsetr = (int)(ImGui::GetStyle().ScrollbarSize + (ImGui::GetStyle().ItemSpacing.x * 2) + (editorUtility->textWidth * 4));
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - sliderOffsetr);
			ImGui::SliderFloat("##ObjectDetail_SizeSlider", &selectorIconSize, selectorIconMinSize, selectorIconMaxSize, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			ImVec2 nowCursor = ImGui::GetCursorPos();
			ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
			float frameBorder = ImGui::GetStyle().FrameBorderSize;
			editorPositionCal->Update(ImGui::GetWindowWidth(), ImGui::GetWindowHeight(), selectorIconSize, selectorIconSize,
				nowCursor.x, nowCursor.y, itemSpacing.x, itemSpacing.y, frameBorder, editorUtility->textWidth, editorUtility->textHeight);

			std::string name;
			std::string subName;
			bool res = false;

			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
			if (ImGui::ColorButton("NullPtrButton", ImVec4(0.15f, 0.15f, 0.15f, 1), ImGuiColorEditFlags_NoTooltip,
				ImVec2(selectorIconSize, selectorIconSize)))
			{
				select = nullptr;
				res = true;
			}

			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(0)));
			name = "None";
			int maxTextCount = editorPositionCal->GetSameLineMaxTextCount();
			int lineCount = 0;
			while ((int)name.size() > maxTextCount)
			{
				++lineCount;
				subName = name.substr(maxTextCount, name.size());
				ImGui::Text(name.substr(0, maxTextCount).c_str());
				ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(),
					editorPositionCal->GetTextPositionY(lineCount)));
				name = subName;
			}
			ImGui::Text(name.c_str());
			editorPositionCal->Next();

			uint count = previewGroup->GetPreviewSceneCount();
			for (uint i = 0; i < count; ++i)
			{
				JPreviewScene* nowPreviewScene = previewGroup->GetPreviewScene(i);
				JResourceObject* nowObject = nowPreviewScene->GetResouceObject();
				const J_OBJECT_FLAG flag = nowObject->GetFlag();
				if ((flag & OBJECT_FLAG_HIDDEN) > 0)
					continue;

				ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
				/*if (ImGui::ImageButton((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(nowPreviewScene->GetPreviewCamera(0)->GetRsSrvHeapIndex()).ptr,
					ImVec2(selectorIconSize, selectorIconSize)))
				{
					*select = nowObject;
					res = true;
				}*/
				name = editorPositionCal->GetString(nowObject->GetName());

				ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(0)));
				int maxTextCount = editorPositionCal->GetSameLineMaxTextCount();
				int lineCount = 0;
				while ((int)name.size() > maxTextCount)
				{
					++lineCount;
					subName = name.substr(maxTextCount, name.size());
					ImGui::Text(name.substr(0, maxTextCount).c_str());
					ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(),
						editorPositionCal->GetTextPositionY(lineCount)));
					name = subName;
				}
				ImGui::Text(name.c_str());
				editorPositionCal->Next();
			}
			ImGui::End();

			if (!selectorWindowKey)
				selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			return res;
		}
		void JObjectDetail::SearchSelectorEvVec()
		{
			const uint evVecCount = (uint)selectorEvVec.size();
			for (uint i = 0; i < evVecCount; ++i)
			{
				if (selectorEvVec[i] == nowSelectorState)
					continue;

				if (selectorEvVec[i] != SELECTOR_STATE::CLOSE)
				{
					if (nowSelectorState != SELECTOR_STATE::CLOSE)
					{
						JResourceManager::Instance().ClearPreviewGroup(previewGroup);
						nowSelectorState = SELECTOR_STATE::CLOSE;
						selectorWindowKey = false;
					}

					if (OpenSelectorWindow(selectorEvVec[i]))
					{
						nowSelectorState = selectorEvVec[i];
						selectorWindowKey = true;
					}
				}
				else
				{
					JResourceManager::Instance().ClearPreviewGroup(previewGroup);
					nowSelectorState = SELECTOR_STATE::CLOSE;
					selectorWindowKey = false;
				}
			}
			selectorEvVec.clear();
		}
		bool JObjectDetail::OpenSelectorWindow(const SELECTOR_STATE selectorState)
		{
			J_RESOURCE_TYPE matchResourceType = J_RESOURCE_TYPE::NONE;
			switch (selectorState)
			{
			case JObjectDetail::SELECTOR_STATE::OPEN_MESH:
				matchResourceType = J_RESOURCE_TYPE::MESH;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_MATERIAL:
				matchResourceType = J_RESOURCE_TYPE::MATERIAL;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_ALBEDO:
				matchResourceType = J_RESOURCE_TYPE::TEXTURE;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_NORMAL:
				matchResourceType = J_RESOURCE_TYPE::TEXTURE;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_HEIGHT:
				matchResourceType = J_RESOURCE_TYPE::TEXTURE;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_ROUGHNESS:
				matchResourceType = J_RESOURCE_TYPE::TEXTURE;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_TEXTURE_AMBIENT:
				matchResourceType = J_RESOURCE_TYPE::TEXTURE;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_ANIMATION_CONTROLLER:
				matchResourceType = J_RESOURCE_TYPE::ANIMATION_CONTROLLER;
				break;
			case JObjectDetail::SELECTOR_STATE::OPEN_SKELETON:
				matchResourceType = J_RESOURCE_TYPE::SKELETON;
				break;
			default:
				matchResourceType = J_RESOURCE_TYPE::NONE;
				break;
			}

			switch (matchResourceType)
			{
			case J_RESOURCE_TYPE::ANIMATION_CLIP:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JAnimationClip>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			case J_RESOURCE_TYPE::MESH:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JMeshGeometry>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			case J_RESOURCE_TYPE::MATERIAL:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JMaterial>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			case J_RESOURCE_TYPE::TEXTURE:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JTexture>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			case J_RESOURCE_TYPE::SKELETON:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JSkeletonAsset>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			case J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st =
					JResourceManager::Instance().GetResourceVectorHandle<JAnimationController>(count);
				for (uint i = 0; i < count; ++i)
				{
					if (((*(st + i))->GetFlag() & OBJECT_FLAG_HIDDEN) == 0)
						JResourceManager::Instance().CreatePreviewScene(previewGroup, *(st + i), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				}
				return true;
			}
			default:
				if (selectorState == SELECTOR_STATE::OPEN_COMPONENT)
				{
					//수정필요
					//Craeet JComponent Icon
					return true;
				}
				else
					return false;
			}
		}
		void JObjectDetail::Spacing()
		{
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
		}
		void JObjectDetail::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)
		{
			if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
				return;
			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
			{
				if (previewGroup->GetPreviewSceneCount() != 0)
					selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			}
			else if (eventType == J_EDITOR_EVENT::SELECT_RESOURCE)
				selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			else if (eventType == J_EDITOR_EVENT::SELECT_GAMEOBJECT)
				selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
			else if (eventType == J_EDITOR_EVENT::DESELECT_GAMEOBJECT)
				selectorEvVec.push_back(SELECTOR_STATE::CLOSE);
		}
	}
}