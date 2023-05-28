#include"JObjectExplorer.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorStringMap.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"   
#include"../../../Helpers/JEditorRenameHelper.h"
#include"../../../Helpers/JEditorSearchBarHelper.h"
#include"../../../../Utility/JCommonUtility.h"    
#include"../../../../Object/Component/JComponentCreator.h"
#include"../../../../Object/GameObject/JGameObject.h"   
#include"../../../../Object/GameObject/JGameObjectCreator.h"
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
		class JObjectExplorerCreationImpl
		{
		private:
			using GameObjectCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<const size_t, const J_DEFAULT_SHAPE>>;
			using ModelCreationInteface = JEditorCreationRequestor<JEditorObjectCreateInterface<const size_t, const size_t>>;
			using DestructionInterface = JEditorDestructionRequestor;
		public:
			using GameObjectCanCreateF = GameObjectCreationInterface::CreateInteface::CanCreateF;
			using GameObjectCreateF = GameObjectCreationInterface::CreateInteface::ObjectCreateF;
			using ModelCanCreateF = ModelCreationInteface::CreateInteface::CanCreateF;
			using ModelCreateF = ModelCreationInteface::CreateInteface::ObjectCreateF;
		public:
			using DataHandleStructure = GameObjectCreationInterface::DataHandleStructure;
			using NotifyPtr = GameObjectCreationInterface::NotifyPtr;
		public:
			DataHandleStructure dS;
		public:
			GameObjectCreationInterface gameObject;
			ModelCreationInteface model;
			DestructionInterface destructuion;
		public:
			using RequestGObjCreationEvF = Core::JSFunctorType<void, JObjectExplorer*, J_DEFAULT_SHAPE>;
			using RequestModelCreationEvF = Core::JSFunctorType<void, JObjectExplorer*, JUserPtr<JGameObject>, JUserPtr<JMeshGeometry>>;
			using RequestDestructionEvF = Core::JSFunctorType<void, JObjectExplorer*>;
		public:
			std::unique_ptr<RequestGObjCreationEvF::Functor> reqGObjCreationEvF;
			std::unique_ptr<RequestModelCreationEvF::Functor> reqModelCreationEvF;
			std::unique_ptr<RequestDestructionEvF::Functor> reqDestructionEvF;
		public:
			JObjectExplorerCreationImpl(RequestGObjCreationEvF::Ptr reqGObjCreationEvPtr,
				RequestModelCreationEvF::Ptr reqModelCreationEvPtr,
				RequestDestructionEvF::Ptr reqDestructionEvPtr)
			{
				reqGObjCreationEvF = std::make_unique<RequestGObjCreationEvF::Functor>(reqGObjCreationEvPtr);
				reqModelCreationEvF = std::make_unique<RequestModelCreationEvF::Functor>(reqModelCreationEvPtr);
				reqDestructionEvF = std::make_unique<RequestDestructionEvF::Functor>(reqDestructionEvPtr);
			}
			~JObjectExplorerCreationImpl()
			{
				dS.Clear();
			}
		};

		class JObjectExplorerSettingImpl
		{
		public:
			using ChangeParentF = Core::JSFunctorType<void, JObjectExplorer*, JUserPtr<JGameObject>, JUserPtr<JGameObject>>;
			using RenameF = Core::JSFunctorType<void, JObjectExplorer*>;
		public:
			std::unique_ptr<ChangeParentF::Functor> changeParentF;
			std::unique_ptr<RenameF::Functor> renameF;
		};


		JObjectExplorer::JObjectExplorer(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>();
			renameHelper = std::make_unique<JEditorRenameHelper>();
			searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);

			InitializeCreationImpl();
			InitializeSettingImpl();

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

			using RequestGObjCreationEvF = JObjectExplorerCreationImpl::RequestGObjCreationEvF;
			using RequestDestructionEvF = JObjectExplorerCreationImpl::RequestDestructionEvF;
			using RenameF = JObjectExplorerSettingImpl::RenameF;

			createCubeNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE));
			createGridNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID));
			createCyilinderNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER));
			createSphereNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE));
			createQuadNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD));
			createEmptyNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY));

			destroyNode->RegisterSelectBind(std::make_unique<RequestDestructionEvF::CompletelyBind>(*creationImpl->reqDestructionEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));
			renameNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*settingImpl->renameF, this));
			renameNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));

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
			creationImpl.reset();
			settingImpl.reset();
		}
		void JObjectExplorer::InitializeCreationImpl()
		{
			if (creationImpl != nullptr)
				return;

			auto requestCreateGObjLam = [](JObjectExplorer* explorer, J_DEFAULT_SHAPE shapeType)
			{
				if (!explorer->root.IsValid())
					return;
				 
				JUserPtr<JGameObject> parent = Core::ConvertChildUserPtr<JGameObject>(explorer->GetHoveredObject());
				if(parent == nullptr)
					parent = explorer->root;
				
				JEditorCreationHint creationHint = JEditorCreationHint(explorer,
					false, true, false, true,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(parent->GetOwnerScene()),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, explorer->GetClearTaskFunctor());
				 
				JObjectExplorerCreationImpl* impl = explorer->creationImpl.get();
				impl->gameObject.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint, parent->GetGuid(), std::move(shapeType));
			};
			auto requestCreateModelLam = [](JObjectExplorer* explorer, JUserPtr<JGameObject> parent, JUserPtr<JMeshGeometry> mesh)
			{
				if (!explorer->root.IsValid() || !mesh.IsValid())
					return;

				if (!parent.IsValid())
					parent = explorer->root;

				JEditorCreationHint creationHint = JEditorCreationHint(explorer,
					false, true, false, true,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(parent->GetOwnerScene()),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, explorer->GetClearTaskFunctor());

				JObjectExplorerCreationImpl* impl = explorer->creationImpl.get();
				impl->model.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint, parent->GetGuid(), mesh->GetGuid());
			};
			auto requestDestroyLam = [](JObjectExplorer* explorer)
			{
				if (!explorer->root.IsValid())
					return;

				std::vector<JUserPtr<Core::JIdentifier>> objVec = explorer->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(explorer,
					false, true, false, true,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(explorer->root->GetOwnerScene()),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, explorer->GetClearTaskFunctor());

				JObjectExplorerCreationImpl* impl = explorer->creationImpl.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creationImpl = std::make_unique<JObjectExplorerCreationImpl>(requestCreateGObjLam, requestCreateModelLam, requestDestroyLam);

			auto canCreationGobjLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const J_DEFAULT_SHAPE shapeType)
			{
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (ownerPtr == nullptr)
					return false;

				auto rawPtr = Core::GetRawPtr(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				if (ownerPtr->GetTypeInfo().IsChildOf<JScene>() && rawPtr!= nullptr)
					return static_cast<JGameObject*>(rawPtr)->GetOwnerScene()->GetGuid() == ownerPtr->GetGuid();
				else
					return false;
			};
			auto creationGobjLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const J_DEFAULT_SHAPE shapeType)
			{
				auto parentUserPtr = Core::GetUserPtr<JGameObject>(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				JGCI::CreateShape(parentUserPtr, guid, OBJECT_FLAG_NONE, shapeType);
			};

			auto canCreationModelLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const size_t meshGuid)
			{
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (ownerPtr == nullptr)
					return false;

				auto parentRawPtr = Core::GetRawPtr(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				auto meshRawPtr = Core::SearchRawPtr(JMeshGeometry::StaticTypeInfo(), meshGuid);
				if (ownerPtr->GetTypeInfo().IsChildOf<JScene>() && parentRawPtr != nullptr && meshRawPtr != nullptr)
					return static_cast<JGameObject*>(parentRawPtr)->GetOwnerScene()->GetGuid() == ownerPtr->GetGuid();
				else
					return false;
			};
			auto creationModelLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const size_t meshGuid)
			{
				auto parentUserPtr = Core::GetUserPtr<JGameObject>(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				auto meshUserPtr = Core::SearchUserPtr<JMeshGeometry>(JMeshGeometry::StaticTypeInfo(), meshGuid);
				JGCI::CreateModel(parentUserPtr, guid, OBJECT_FLAG_NONE, meshUserPtr);
			};

			creationImpl->gameObject.GetCreationInterface()->RegisterCanCreationF(canCreationGobjLam);
			creationImpl->gameObject.GetCreationInterface()->RegisterObjectCreationF(creationGobjLam);
			creationImpl->model.GetCreationInterface()->RegisterCanCreationF(canCreationModelLam);
			creationImpl->model.GetCreationInterface()->RegisterObjectCreationF(creationModelLam);
		}
		void JObjectExplorer::InitializeSettingImpl()
		{
			auto changeParentLam = [](JObjectExplorer* objEx, JUserPtr<JGameObject> obj, JUserPtr<JGameObject> newP)
			{
				obj->ChangeParent(newP);
				objEx->SetModifiedBit(obj->GetOwnerScene(), true);
			};
			auto renameLam = [](JObjectExplorer* objEx)
			{
				objEx->renameHelper->Activate(objEx->GetHoveredObject());
				objEx->SetModifiedBit(static_cast<JGameObject*>(objEx->GetHoveredObject().Get())->GetOwnerScene(), true);
			};
			settingImpl = std::make_unique<JObjectExplorerSettingImpl>();
			settingImpl->changeParentF = std::make_unique<JObjectExplorerSettingImpl::ChangeParentF::Functor>(changeParentLam);
			settingImpl->renameF = std::make_unique<JObjectExplorerSettingImpl::RenameF::Functor>(renameLam);
		}
		J_EDITOR_WINDOW_TYPE JObjectExplorer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::OBJECT_EXPLORER;
		}
		void JObjectExplorer::Initialize(JUserPtr<JGameObject> newRoot) noexcept
		{
			root = newRoot;
			renameHelper->Clear();
			searchBarHelper->ClearInputBuffer();
			explorerPopup->SetOpen(false);
		}
		void JObjectExplorer::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && root.IsValid())
			{
				UpdateMouseClick();
				searchBarHelper->UpdateSearchBar();
				BuildObjectExplorer();
			}
			CloseWindow();
		}
		void JObjectExplorer::BuildObjectExplorer()
		{
			ObjectExplorerOnScreen(root, searchBarHelper->HasInputData());
			UpdatePopup(PopupSetting(explorerPopup.get(), editorString.get())); 
			ImGui::SameLine();
		}
		void JObjectExplorer::ObjectExplorerOnScreen(const JUserPtr<JGameObject>& gObj, const bool isAcivatedSearch)
		{
			//ImGuiTreeNodeFlags_Selected
			bool isNodeOpen = false;
			bool isRenameActivaetd = renameHelper->IsActivated() && renameHelper->IsRenameTar(gObj->GetGuid());

			//selected mark를 이용하면 자식 gameObject를 참조하는 node까지 mark되어버리므로
			//이는 Explorer에서 기대하는 바가 아니므로 직접 클릭한 gameObject만 저장되는
			//SelectedMap을 참조해서 mark
			bool isSelected = IsSelectedObject(gObj->GetGuid()); 

			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;

			//if(!isSelected && gObj->IsSelectedbyEditor())
			//	RequestPushSelectObject(Core::GetUserPtr(gObj));

			std::string name = JCUtil::WstrToU8Str(gObj->GetName());
			bool canOnScreen = searchBarHelper->CanSrcNameOnScreen(name);
			if (canOnScreen)
			{
				if (isRenameActivaetd)
				{
					isNodeOpen = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetCurrentWindow()->GetID((name + "##TreeNode").c_str()), baseFlags);
					renameHelper->Update(isNodeOpen);
				}
				else
				{
					if (isSelected)
						SetTreeNodeColor(GetSelectedColorFactor());
					if (isAcivatedSearch)
						ImGui::SetNextItemOpen(true);
					isNodeOpen = JImGuiImpl::TreeNodeEx((name + "##TreeNode").c_str(), baseFlags);
					if (isSelected)
						SetTreeNodeColor(GetSelectedColorFactor() * -1);
					if (isNodeOpen)
					{
						if (ImGui::IsItemHovered())
							SetHoveredObject(gObj);

						if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
						{
							RequestPushSelectObject(gObj);
							SetContentsClick(true);
						}

						TryBeginDragging(gObj);
						JUserPtr<Core::JIdentifier> dragResult = TryGetDraggingTarget();

						if (dragResult.IsValid())
						{
							Core::JTypeInfo& typeInfo = dragResult->GetTypeInfo();
							JObject* obj = typeInfo.IsChildOf<JObject>() ? static_cast<JObject*>(dragResult.Get()) : nullptr;
							if (obj != nullptr && obj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
							{
								JUserPtr<JGameObject> selectedObj; 
								selectedObj.ConnnectChild(dragResult);

								using ChangeParentF = JObjectExplorerSettingImpl::ChangeParentF;
								 
								std::string taskName = "Change parent";
								std::string taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " + selectedObj->GetName() + L" to " + gObj->GetName());

								auto doBind = std::make_unique<ChangeParentF::CompletelyBind>(*settingImpl->changeParentF, this, JUserPtr<JGameObject>(selectedObj), JUserPtr<JGameObject>(gObj));
								auto undoBind = std::make_unique<ChangeParentF::CompletelyBind>(*settingImpl->changeParentF, this, JUserPtr<JGameObject>(selectedObj), selectedObj->GetParent());
								auto evStruct = std::make_unique<JEditorTSetBindFuncEvStruct>(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind));

								AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
							}
							else if (obj != nullptr && obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
							{
								JUserPtr<JMeshGeometry> sMesh;
								sMesh.ConnnectChild(dragResult);
								if (sMesh.IsValid())
									creationImpl->reqModelCreationEvF->Invoke(this, gObj, sMesh);
							}
						}
					}
				}
			}
			if (isNodeOpen || isAcivatedSearch)
			{
				const uint childrenCount = gObj->GetChildrenCount();
				for (uint i = 0; i < childrenCount; ++i)
				{
					JUserPtr<JGameObject> child = gObj->GetChild(i);
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
		void JObjectExplorer::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> listenEvTypeVec{ J_EDITOR_EVENT::MOUSE_CLICK };
			AddEventListener(*JEditorEvent::EvInterface(), GetGuid(), listenEvTypeVec);
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
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				explorerPopup->SetOpen(false);

			else if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && eventStruct->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStruct);
				JUserPtr< Core::JIdentifier> gameObj = evstruct->GetLastMatchedTypeObject(JGameObject::StaticTypeInfo());
				if (gameObj.IsValid())
				{
					ClearSelectedObject();
					PushSelectedObject(gameObj);				 		
				}
			}
		}
	}
}