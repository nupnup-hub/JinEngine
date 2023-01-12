#include"JObjectExplorer.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"   
#include"../../../Utility/JEditorSearchBarHelper.h"
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
		JObjectExplorer::JObjectExplorer(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			editorString = std::make_unique<JEditorString>();
			renameHelper = std::make_unique<JEditorRenameHelper>();
			searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);

			//PopupInit
			std::unique_ptr<JEditorPopupNode> explorerPopupRootNode =
				std::make_unique<JEditorPopupNode>("JObjectExplorer Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createGameObjectNode =
				std::make_unique<JEditorPopupNode>("Create GameObject", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, explorerPopupRootNode.get(), true);
			editorString->AddString(createGameObjectNode->GetNodeId(), { "Create GameObject", u8"게임오브젝트 생성" });
			editorString->AddString(createGameObjectNode->GetTooltipId(), { "Create selected gameObject's child in the scene", u8"씬내에 선택한 게임오브젝에 자식을 생성합니다." });

			std::unique_ptr<JEditorPopupNode> createCubeNode =
				std::make_unique<JEditorPopupNode>("Cube", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createCubeNode->GetNodeId(), { "Cube", u8"정육면체" });

			std::unique_ptr<JEditorPopupNode> createGridNode =
				std::make_unique<JEditorPopupNode>("Grid", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createGridNode->GetNodeId(), { "Grid", u8"격자판" });

			std::unique_ptr<JEditorPopupNode> createCyilinderNode =
				std::make_unique<JEditorPopupNode>("Cyilinder", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createCyilinderNode->GetNodeId(), { "Cyilinder", u8"원기둥" });

			std::unique_ptr<JEditorPopupNode> createSphereNode =
				std::make_unique<JEditorPopupNode>("Sphere", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createSphereNode->GetNodeId(), { "Sphere", u8"구체" });

			std::unique_ptr<JEditorPopupNode> createQuadNode =
				std::make_unique<JEditorPopupNode>("Quad", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createQuadNode->GetNodeId(), { "Quad", u8"사각형" });

			std::unique_ptr<JEditorPopupNode> createEmptyNode =
				std::make_unique<JEditorPopupNode>("Empty", J_EDITOR_POPUP_NODE_TYPE::LEAF, createGameObjectNode.get());
			editorString->AddString(createEmptyNode->GetNodeId(), { "Empty", u8"빈 오브젝트" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), true);
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy JGameObject", u8"게임오브젝트 삭제" });
			editorString->AddString(destroyNode->GetTooltipId(), { "Destroy selected gameObject in the scene", u8"씬내에 선택한 게임오브젝트를 삭제합니다." });

			std::unique_ptr<JEditorPopupNode> renameNode =
				std::make_unique<JEditorPopupNode>("Rename JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), false);
			editorString->AddString(renameNode->GetNodeId(), { "Rename", u8"새이름" });

			auto destroyLam = [](DataHandleStructure& dS, Core::JDataHandle& dH, const size_t guid)
			{
				auto owner = Core::JIdentifier::ReleaseInstance<JGameObject>(guid);
				if (owner.IsValid())
				{
					Core::JDataHandle newHandle = dS.Add(Core::JOwnerPtr<JGameObject>::ConvertChildType(std::move(owner)));
					dS.TransitionHandle(newHandle, dH);
				}
			};
			auto undoDestroyLam = [](DataHandleStructure& dS, Core::JDataHandle& dH)
			{
				auto owner = dS.Release(dH);
				if (owner.IsValid())
					Core::JIdentifier::AddInstance(std::move(owner));
			};
			auto changeParentLam = [](Core::JUserPtr<JGameObject> obj, Core::JUserPtr<JGameObject> newP) {obj->ChangeParent(newP.Get()); };
			auto renameLam = [](JObjectExplorer* objEx) {objEx->renameHelper->Activate(objEx->selectedObject); };
			
			createF = std::make_unique<CreateGameObjectFunctor>(&JObjectExplorer::CreateGameObject, this);
			createModelF = std::make_unique<CreateModelFunctor>(&JObjectExplorer::CreateModel, this);
			destroyF = std::make_unique<DestroyGameObjectFunctor>(destroyLam);
			undoDestroyF = std::make_unique<UndoDestroyGameObjectFunctor>(undoDestroyLam);
			changeParentF = std::make_unique< ChangeParentF::Functor>(changeParentLam);
			regCreateGobjF = std::make_unique<RegisterCreateGEvF::Functor>(&JObjectExplorer::RegisterCreateGameObjectEv, this);
			regDestroyGobjF = std::make_unique<RegisterDestroyGEvF::Functor>(&JObjectExplorer::RegisterDestroyGameObjectEv, this);
			renameF = std::make_unique<RenameF::Functor>(renameLam);

			createCubeNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE));
			createGridNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID));
			createCyilinderNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER));
			createSphereNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE));
			createQuadNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD));
			createEmptyNode->RegisterSelectBind(std::make_unique<RegisterCreateGEvF::CompletelyBind>(*regCreateGobjF, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY));

			destroyNode->RegisterSelectBind(std::make_unique<RegisterDestroyGEvF::CompletelyBind>(*regDestroyGobjF));
			renameNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*renameF, this));

			explorerPopup = std::make_unique<JEditorPopupMenu>("explorerPopup", std::move(explorerPopupRootNode));
			explorerPopup->AddPopupNode(std::move(createGameObjectNode));
			explorerPopup->AddPopupNode(std::move(createCubeNode));
			explorerPopup->AddPopupNode(std::move(createGridNode));
			explorerPopup->AddPopupNode(std::move(createCyilinderNode));
			explorerPopup->AddPopupNode(std::move(createSphereNode));
			explorerPopup->AddPopupNode(std::move(createQuadNode));
			explorerPopup->AddPopupNode(std::move(createEmptyNode));
			explorerPopup->AddPopupNode(std::move(destroyNode));
			explorerPopup->AddPopupNode(std::move(renameNode));
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
			renameHelper->Clear();
			searchBarHelper->ClearInputBuffer();
			explorerPopup->SetOpen(false);
		}
		void JObjectExplorer::UpdateWindow(const JEditorWindowUpdateCondition& condition)
		{
			EnterWindow(condition, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && root.IsValid())
			{
				auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				const bool isValidGameObject = selected.IsValid() && selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT;
				const bool canSetGameObject = !selectedObject.IsValid() || selectedObject->GetGuid() != selected->GetGuid();
				if (isValidGameObject && canSetGameObject)
					selectedObject = Core::GetUserPtr(static_cast<JGameObject*>(selected.Get()));

				UpdateMouseClick();
				searchBarHelper->UpdateSearchBar(GetName(), false);
				BuildObjectExplorer();
			}
			CloseWindow();
		}
		void JObjectExplorer::BuildObjectExplorer()
		{
			ObjectExplorerOnScreen(root.Get(), searchBarHelper->HasInputData());
			if (explorerPopup->IsOpen())
				explorerPopup->ExecutePopup(editorString.get());

			explorerPopup->Update();
			ImGui::SameLine();
		}
		void JObjectExplorer::ObjectExplorerOnScreen(JGameObject* gObj, const bool isAcivatedSearch)
		{
			//ImGuiTreeNodeFlags_Selected
			bool isNodeOpen = false;
			bool isRenameActivaetd = renameHelper->IsActivated() && renameHelper->IsRenameTar(gObj->GetGuid());
			bool isSelected = selectedObject.IsValid() && gObj->GetGuid() == selectedObject->GetGuid();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;

			std::string name = JCUtil::WstrToU8Str(gObj->GetName());
			bool canOnScreen = searchBarHelper->CanSrcNameOnScreen(name);
			if (canOnScreen)
			{ 
				if (isRenameActivaetd)
				{
					isNodeOpen = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetCurrentWindow()->GetID((name + "##TreeNode").c_str()), baseFlags);
					renameHelper->Update(GetName(), isNodeOpen);
				}
				else
				{
					if (isSelected)
						SetTreeNodeColor(JImGuiImpl::GetTreeDeepFactor());
					if (isAcivatedSearch)
						ImGui::SetNextItemOpen(true);
					isNodeOpen = JImGuiImpl::TreeNodeEx((name + "##TreeNode").c_str(), baseFlags);
					if (isSelected)
						SetTreeNodeColor(-JImGuiImpl::GetTreeDeepFactor());
					if (isNodeOpen)
					{
						if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(gObj)));
				 
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(gObj)));
							std::string selectName = name;
							JImGuiImpl::Text(selectName);
							ImGui::SetDragDropPayload(selectName.c_str(), JEditorPageShareData::GetDragGuidPtr(GetOwnerPageType()), sizeof(int));
							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginDragDropTarget())
						{
							auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
							if (selected.IsValid() && !selected->HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNEDITABLE))
							{
								const std::string itemName = JCUtil::WstrToU8Str(selected->GetName());
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(itemName.c_str(), ImGuiDragDropFlags_None);

								if (selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT && !ImGui::IsMouseDragging(0))
								{
									Core::JUserPtr<JGameObject> selectedObj;
									selectedObj.ConnnectBaseUser(selected);

									auto doBind = std::make_unique<ChangeParentF::CompletelyBind>(*changeParentF, Core::JUserPtr(selectedObj), Core::GetUserPtr(gObj));
									auto undoBind = std::make_unique<ChangeParentF::CompletelyBind>(*changeParentF, Core::JUserPtr(selectedObj), Core::GetUserPtr(selectedObj->GetParent()));
									auto evStruct = std::make_unique<JEditorTSetBindFuncEvStruct>("Change Parent", GetOwnerPageType(), std::move(doBind), std::move(undoBind));

									AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
								}
								else if (selected->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT && !ImGui::IsMouseDragging(0))
								{
									Core::JUserPtr<JMeshGeometry> sMesh;
									sMesh.ConnnectBaseUser(selected);
									if (sMesh.IsValid())
									{
										size_t guid = Core::MakeGuid();
										//using CreateModelBind = Core::JBindHandle<CreateModelFunctor, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
										auto doBind = Core::CTaskUptr<CreateModelBind>(*createModelF, Core::GetUserPtr(gObj), Core::JUserPtr{ sMesh }, std::move(guid));
										auto undoBind = Core::CTaskUptr<DestroyGameObjectBind>(*destroyF, std::move(guid));

										auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateModelEvStruct>
											("Create model object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
										AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
									}
								}
							}
							ImGui::EndDragDropTarget();
						}
					}
				}	 
			}
			if (isNodeOpen || isAcivatedSearch)
			{
				const uint childrenCount = gObj->GetChildrenCount();
				for (uint i = 0; i < childrenCount; ++i)
				{
					JGameObject* child = gObj->GetChild(i);
					if ((child->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
						continue;

					ObjectExplorerOnScreen(child, isAcivatedSearch);
				}
				if (isRenameActivaetd)
					ImGui::Unindent();
				else if (isNodeOpen)
					JImGuiImpl::TreePop();
			} 
		}
		void JObjectExplorer::RegisterCreateGameObjectEv(J_DEFAULT_SHAPE shapeType)
		{
			auto parent = selectedObject;
			if (!parent.IsValid())
				parent = root;

			size_t guid = Core::MakeGuid();
			auto doBind = Core::CTaskUptr<CreateGameObjectBind>(*createF, Core::JUserPtr{ parent }, std::move(guid), std::move(shapeType));
			auto undoBind = Core::CTaskUptr<DestroyGameObjectBind>(*destroyF, std::move(guid));

			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateGameObjectEvStruct>
				("Create game object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JObjectExplorer::RegisterDestroyGameObjectEv()
		{
			auto doBind = Core::CTaskUptr<DestroyGameObjectBind>(*destroyF, selectedObject->GetGuid());
			auto undoBind = Core::CTaskUptr<UndoDestroyGameObjectBind>(*undoDestroyF);

			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<DestroyGameObjectEvStruct>
				("Destroy game object", GetOwnerPageType(), std::move(doBind), std::move(undoBind), dataStructure));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JObjectExplorer::CreateGameObject(DataHandleStructure& dS,
			Core::JDataHandle& dH,
			Core::JUserPtr<JGameObject> p,
			const size_t guid,
			const J_DEFAULT_SHAPE shapeType)
		{ 
			if (dS.IsValidHandle(dH))
				Core::JIdentifier::AddInstance(dS.Release(dH));
			else if (p.IsValid())
				JGFU::CreateShape(*p, guid, OBJECT_FLAG_NONE, shapeType);
		}
		void JObjectExplorer::CreateModel(DataHandleStructure& dS,
			Core::JDataHandle& dH,
			Core::JUserPtr<JGameObject> p,
			Core::JUserPtr<JMeshGeometry> m,
			const size_t guid)
		{
			if (dS.IsValidHandle(dH))
				Core::JIdentifier::AddInstance(dS.Release(dH));
			else if (p.IsValid() && m.IsValid())
				JGFU::CreateModel(*p, guid, OBJECT_FLAG_NONE, m.Get());
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
			renameHelper->Clear();
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