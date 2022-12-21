#include"JObjectExplorer.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopup.h"
#include"../../../Popup/JEditorPopupNode.h"   
#include"../../../../Utility/JCommonUtility.h"   
#include"../../../../Object/Component/JComponentFactoryUtility.h"
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/GameObject/JGameObjectFactory.h"
#include"../../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../../Core/Identity/JIdentifier.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../../Lib/imgui/imgui.h"  

namespace JinEngine
{
	namespace Editor
	{
		using CreateGameObjectHelperFunctor = Core::JFunctor<void, Core::JOwnerPtr<JGameObject>, Core::JUserPtr<JGameObject>, const size_t, const J_DEFAULT_SHAPE>;
		static std::unique_ptr< CreateGameObjectHelperFunctor> cGObjHelperF;

		//std::unique_ptr<
		JObjectExplorer::JObjectExplorer(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			if (cGObjHelperF == nullptr)
			{
				auto CreateGameObjectHelperLam = [](Core::JOwnerPtr<JGameObject> owner,
					Core::JUserPtr<JGameObject> parent,
					const size_t guid,
					const J_DEFAULT_SHAPE shapeType)
				{
					if (owner.IsValid())
						Core::JIdentifier::AddInstance(std::move(owner));
					else
					{
						if (parent.IsValid())
							JGFU::CreateShape(*(parent.Get()), guid, OBJECT_FLAG_NONE, shapeType);
					}
				};
				cGObjHelperF = std::make_unique<CreateGameObjectHelperFunctor>(CreateGameObjectHelperLam);
			}

			//JEditorString Init
			editorString = std::make_unique<JEditorString>();

			//PopupInit
			std::unique_ptr<JEditorPopupNode> explorerPopupRootNode =
				std::make_unique<JEditorPopupNode>("JObjectExplorer Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createGameObjectNode =
				std::make_unique<JEditorPopupNode>("Create GameObject", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, explorerPopupRootNode.get(), true);
			editorString->AddString(createGameObjectNode->GetNodeId(), { "Create GameObject", u8"게임오브젝트 생성" });
			editorString->AddString(createGameObjectNode->GetTooltipId(), { "Create selected gameObject's child in the scene", u8"씬내에 선택한 게임오브젝에 자식을 생성합니다." });

			std::unique_ptr<JEditorPopupNode> createCubeNode =
				std::make_unique<JEditorPopupNode>("Cube", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createCubeNode->GetNodeId(), { "Cube", u8"정육면체" });

			std::unique_ptr<JEditorPopupNode> createGridNode =
				std::make_unique<JEditorPopupNode>("Grid", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createGridNode->GetNodeId(), { "Grid", u8"격자판" });

			std::unique_ptr<JEditorPopupNode> createCyilinderNode =
				std::make_unique<JEditorPopupNode>("Cyilinder", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createCyilinderNode->GetNodeId(), { "Cyilinder", u8"원기둥" });

			std::unique_ptr<JEditorPopupNode> createSphereNode =
				std::make_unique<JEditorPopupNode>("Sphere", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createSphereNode->GetNodeId(), { "Sphere", u8"구체" });

			std::unique_ptr<JEditorPopupNode> createQuadNode =
				std::make_unique<JEditorPopupNode>("Quad", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createQuadNode->GetNodeId(), { "Quad", u8"사각형" });

			std::unique_ptr<JEditorPopupNode> createEmptyNode =
				std::make_unique<JEditorPopupNode>("Empty", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
			editorString->AddString(createEmptyNode->GetNodeId(), { "Empty", u8"빈 오브젝트" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, explorerPopupRootNode.get(), true);
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy JGameObject", u8"게임오브젝트 삭제" });
			editorString->AddString(destroyNode->GetTooltipId(), { "Destroy selected gameObject in the scene", u8"씬내에 선택한 게임오브젝트를 삭제합니다." });

			std::unique_ptr<JEditorPopupNode> renameNode =
				std::make_unique<JEditorPopupNode>("Rename JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, explorerPopupRootNode.get(), false);
			editorString->AddString(renameNode->GetNodeId(), { "Rename", u8"새이름" });

			auto createCubeLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE);
			};
			auto createGridLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID);
			};
			auto createCyilinderLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER);
			};
			auto createSphereLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
			};
			auto createQuadLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);
			};
			auto createEmptyLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, Core::JUserPtr<JGameObject> p, const size_t guid)
			{
				(*cGObjHelperF)(dS.Release(dH), p, guid, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY);
			};

			createFuncMap.emplace(createCubeNode->GetNodeId(), std::make_unique<CreateGameObjectFunctor>(createCubeLam));
			createFuncMap.emplace(createGridNode->GetNodeId(), std::make_unique< CreateGameObjectFunctor>(createGridLam));
			createFuncMap.emplace(createCyilinderNode->GetNodeId(), std::make_unique< CreateGameObjectFunctor>(createCyilinderLam));
			createFuncMap.emplace(createSphereNode->GetNodeId(), std::make_unique< CreateGameObjectFunctor>(createSphereLam));
			createFuncMap.emplace(createQuadNode->GetNodeId(), std::make_unique< CreateGameObjectFunctor>(createQuadLam));
			createFuncMap.emplace(createEmptyNode->GetNodeId(), std::make_unique< CreateGameObjectFunctor>(createEmptyLam));

			auto destroyLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, const size_t guid)
			{
				auto owner = Core::JIdentifier::ReleaseInstance<JGameObject>(guid);
				if (owner.IsValid())
				{
					Core::JDataHandle newHandle = dS.Add(Core::JOwnerPtr<JGameObject>::ConvertChildType(std::move(owner)));
					dS.TransitionHandle(newHandle, dH);
				}
			};
			destroyT = std::tuple(destroyNode->GetNodeId(), std::make_unique<DestroyGameObjectFunctor>(destroyLam));

			auto renameLam = [](const std::string newName, Core::JUserPtr<JGameObject> obj) {obj->SetName(JCUtil::U8StrToWstr(newName)); };
			renameT = std::tuple(renameNode->GetNodeId(), std::make_unique< RenameFuncF::Functor>(renameLam));

			auto undoDestroyLam = [](DataHandleStructure& dS, Core::JDataHandle& dH)
			{
				auto owner = dS.Release(dH);
				if (owner.IsValid())
					Core::JIdentifier::AddInstance(std::move(owner));
			};
			undoDestroyF = std::make_unique<UndoDestroyGameObjectFunctor>(undoDestroyLam);

			auto createModelLam = [](DataHandleStructure& ds, Core::JDataHandle& dh, Core::JUserPtr<JGameObject> p, Core::JUserPtr<JMeshGeometry> m, const size_t guid)
			{
				auto owner = ds.Release(dh);
				if (owner.IsValid())
					Core::JIdentifier::AddInstance(std::move(owner));
				else
				{
					if (p.IsValid() && m.IsValid())
						JGFU::CreateModel(*p.Get(), guid, OBJECT_FLAG_NONE, m.Get());
				}
			};
			createModelF = std::make_unique<CreateModelFunctor>(createModelLam);

			explorerPopup = std::make_unique<JEditorPopup>("explorerPopup", std::move(explorerPopupRootNode));
			explorerPopup->AddPopupNode(std::move(createGameObjectNode));
			explorerPopup->AddPopupNode(std::move(createCubeNode));
			explorerPopup->AddPopupNode(std::move(createGridNode));
			explorerPopup->AddPopupNode(std::move(createCyilinderNode));
			explorerPopup->AddPopupNode(std::move(createSphereNode));
			explorerPopup->AddPopupNode(std::move(createQuadNode));
			explorerPopup->AddPopupNode(std::move(createEmptyNode));
			explorerPopup->AddPopupNode(std::move(destroyNode));
			explorerPopup->AddPopupNode(std::move(renameNode));

			auto changeParentLam = [](Core::JUserPtr<JGameObject> obj, Core::JUserPtr<JGameObject> newP) {obj->ChangeParent(newP.Get()); };
			changeParentF = std::make_unique< ChangeParentF::Functor>(changeParentLam);

			inputBuff = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
		}
		JObjectExplorer::~JObjectExplorer()
		{
			dataStructure.Clear();
		}
		J_EDITOR_WINDOW_TYPE JObjectExplorer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::OBJECT_EXPLORER;
		}
		void JObjectExplorer::Initialize(Core::JUserPtr<JGameObject> newRoot) noexcept
		{
			root = newRoot;
		}
		void JObjectExplorer::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && root.IsValid())
			{
				auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				const bool isValidGameObject = selected.IsValid() && selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT;
				const bool canSetGameObject = !selectedObject.IsValid() || selectedObject->GetGuid() != selected->GetGuid();
				if (isValidGameObject && canSetGameObject)
					selectedObject.ConnnectBaseUser(selected);

				UpdateMouseClick();
				BuildObjectExplorer();
			}
			CloseWindow();
		}
		void JObjectExplorer::BuildObjectExplorer()
		{
			ObjectExplorerOnScreen(root.Get());
			SetTreeNodeDefaultColor();

			if (explorerPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				explorerPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto func = createFuncMap.find(menuGuid);
					if (func != createFuncMap.end())
					{
						auto parent = selectedObject;
						if (!parent.IsValid())
							parent = root;

						size_t guid = Core::MakeGuid();
						auto doBind = Core::CTaskUptr<CreateGameObjectBind>(*func->second, Core::JUserPtr{ parent }, std::move(guid));
						auto undoBind = Core::CTaskUptr<DestroyGameObjectBind>(*std::get<1>(destroyT), std::move(guid));

						auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateGameObjectEvStruct>
							("Create game object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
						AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
					}
					else if (selectedObject.IsValid() && menuGuid == std::get<0>(destroyT))
					{
						auto doBind = Core::CTaskUptr<DestroyGameObjectBind>(*std::get<1>(destroyT), selectedObject->GetGuid());
						auto undoBind = Core::CTaskUptr<UndoDestroyGameObjectBind>(*undoDestroyF);

						auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<DestroyGameObjectEvStruct>
							("Destroy game object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
						AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
					}
					else if (menuGuid == std::get<0>(renameT))
					{
						renameTar = selectedObject;
						inputBuff->SetBuff(JCUtil::WstrToU8Str(renameTar->GetName())); 
					}
					explorerPopup->SetOpen(false);
				}
			}

			explorerPopup->Update();
			ImGui::SameLine();
		}
		void JObjectExplorer::ObjectExplorerOnScreen(JGameObject* gObj)
		{
			//ImGuiTreeNodeFlags_Selected
			bool isSelected = selectedObject.IsValid() && gObj->GetGuid() == selectedObject->GetGuid();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;
		
			if (renameTar.IsValid() && gObj->GetGuid() == renameTar->GetGuid())
			{  
				const ImVec2 itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();
				if (JImGuiImpl::InputTextSet(GetName(),
					inputBuff.get(),
					ImGuiInputTextFlags_EnterReturnsTrue,
					*std::get<1>(renameT), Core::JUserPtr{ renameTar }))
					renameTar.Clear();
				 
				const ImVec2 itemSize = ImGui::GetItemRectSize();
				if (JImGuiImpl::IsRightMouseClicked() || JImGuiImpl::IsLeftMouseClicked())
				{ 
					if (!JImGuiImpl::IsMouseInRect(itemPos, itemSize))
						renameTar.Clear();
				}

				const uint childrenCount = gObj->GetChildrenCount();
				for (uint i = 0; i < childrenCount; ++i)
				{
					JGameObject* child = gObj->GetChild(i);
					if ((child->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
						continue;

					ObjectExplorerOnScreen(child);
				}
			}
			else
			{
				std::string name = JCUtil::WstrToU8Str(gObj->GetName());
				if (isSelected)
					SetTreeNodeSelectColor();
				bool isNodeOpen = ImGui::TreeNodeEx((name + "##TreeNode").c_str(), baseFlags);
				if (isSelected)
					SetTreeNodeDefaultColor();
				if (isNodeOpen)
				{
					if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
						RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(gObj)));

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(gObj)));
						JImGuiImpl::SetMouseDrag(true);
						std::string selectName = name;
						JImGuiImpl::Text(selectName);
						ImGui::SetDragDropPayload(selectName.c_str(), JEditorPageShareData::GetDragGuidPtr(GetOwnerPageType()), sizeof(int));
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						if (JImGuiImpl::IsDraggingMouse())
						{
							auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
							if (selected.IsValid() && !selected->HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNEDITABLE))
							{
								const std::string itemName = JCUtil::WstrToU8Str(selected->GetName());
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(itemName.c_str(), ImGuiDragDropFlags_None);

								if (selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT && payload)
								{
									Core::JUserPtr<JGameObject> selectedObj;
									selectedObj.ConnnectBaseUser(selected);

									auto doBind = std::make_unique<ChangeParentF::CompletelyBind>(*changeParentF, Core::JUserPtr(selectedObj), Core::GetUserPtr(gObj));
									auto undoBind = std::make_unique<ChangeParentF::CompletelyBind>(*changeParentF, Core::JUserPtr(selectedObj), Core::GetUserPtr(selectedObj->GetParent()));
									auto evStruct = std::make_unique<JEditorTSetBindFuncEvStruct>("Change Parent", GetOwnerPageType(), std::move(doBind), std::move(undoBind));
									
									doBind->Invoke();
									AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
								}
								else if (selected->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT && payload)
								{
									Core::JUserPtr<JMeshGeometry> sMesh;
									sMesh.ConnnectBaseUser(selected);
									if (sMesh.IsValid())
									{
										size_t guid = Core::MakeGuid();
										//using CreateModelBind = Core::JBindHandle<CreateModelFunctor, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
										auto doBind = Core::CTaskUptr<CreateModelBind>(*createModelF, Core::GetUserPtr(gObj), Core::JUserPtr{ sMesh }, std::move(guid));
										auto undoBind = Core::CTaskUptr<DestroyGameObjectBind>(*std::get<1>(destroyT), std::move(guid));

										auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateModelEvStruct>
											("Create model object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
										AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
									}
								}
							}
							JImGuiImpl::SetMouseDrag(false);
						}
						ImGui::EndDragDropTarget();
					}

					const uint childrenCount = gObj->GetChildrenCount();
					for (uint i = 0; i < childrenCount; ++i)
					{
						JGameObject* child = gObj->GetChild(i);
						if ((child->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
							continue;

						ObjectExplorerOnScreen(child);
					}
					ImGui::TreePop();
				}
			}
		}
		void JObjectExplorer::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			AddEventListener(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK);
		}
		void JObjectExplorer::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			RemoveListener(*JEditorEvent::EvInterface(), GetGuid());
		}
		void JObjectExplorer::DoSetUnFocus()noexcept
		{
			JEditorWindow::DoSetUnFocus();
			explorerPopup->SetOpen(false);
			renameTar.Clear();
		}

		void JObjectExplorer::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				explorerPopup->SetOpen(false);
		}
	}
}