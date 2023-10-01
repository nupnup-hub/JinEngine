#include"JObjectExplorer.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../Gui/JGui.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorStringMap.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"   
#include"../../../EditTool/JEditorRenameHelper.h"
#include"../../../EditTool/JEditorSearchBarHelper.h"
#include"../../../../Object/Component/JComponentCreator.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/GameObject/JGameObjectPrivate.h"
#include"../../../../Object/GameObject/JGameObjectCreator.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../../Core/Identity/JIdentifier.h"
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Utility/JCommonUtility.h"     

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			//using NodeColorFactor = int;
			//static NodeColorFactor setNodeColorFactor = 1;
			//static NodeColorFactor offNodeColorFactor = -1;
			static const JVector4<float> actColor = JVector4<float>(0.15f, 0.7f, 0.15f, 0.7f);
			static const JVector4<float> deActColor = JVector4<float>(0.7f, 0.15f, 0.15f, 0.7f);
		}

		class JObjectExplorerCreationImpl
		{
		private:
			using GameObjectCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<const size_t, const J_DEFAULT_SHAPE>>;
			using ModelCreationInteface = JEditorCreationRequestor<JEditorObjectCreateInterface<const size_t, const size_t>>;
			using LightCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<const size_t, const J_LIGHT_TYPE>>;
			using DestructionInterface = JEditorDestructionRequestor;
		public:
			using GameObjectCanCreateF = GameObjectCreationInterface::CreateInteface::CanCreateF;
			using GameObjectCreateF = GameObjectCreationInterface::CreateInteface::ObjectCreateF;
			using ModelCanCreateF = ModelCreationInteface::CreateInteface::CanCreateF;
			using ModelCreateF = ModelCreationInteface::CreateInteface::ObjectCreateF;
			using LightCanCreateF = LightCreationInterface::CreateInteface::CanCreateF;
			using LightCreateF = LightCreationInterface::CreateInteface::ObjectCreateF;
		public:
			using DataHandleStructure = GameObjectCreationInterface::DataHandleStructure;
			using NotifyPtr = GameObjectCreationInterface::NotifyPtr;
		public:
			DataHandleStructure dS;
		public:
			GameObjectCreationInterface gameObject;
			ModelCreationInteface model;
			LightCreationInterface light;
			DestructionInterface destructuion;
		public:
			using RequestGObjCreationEvF = Core::JSFunctorType<void, JObjectExplorer*, J_DEFAULT_SHAPE>;
			using RequestModelCreationEvF = Core::JSFunctorType<void, JObjectExplorer*, JUserPtr<JGameObject>, JUserPtr<JMeshGeometry>>;
			using RequestLightCreationEvF = Core::JSFunctorType<void, JObjectExplorer*, J_LIGHT_TYPE>;
			using RequestDestructionEvF = Core::JSFunctorType<void, JObjectExplorer*>;
		public:
			std::unique_ptr<RequestGObjCreationEvF::Functor> reqGObjCreationEvF;
			std::unique_ptr<RequestModelCreationEvF::Functor> reqModelCreationEvF;
			std::unique_ptr<RequestLightCreationEvF::Functor> reqLightCreationEvF;
			std::unique_ptr<RequestDestructionEvF::Functor> reqDestructionEvF;
		public:
			JObjectExplorerCreationImpl(RequestGObjCreationEvF::Ptr reqGObjCreationEvPtr,
				RequestModelCreationEvF::Ptr reqModelCreationEvPtr,
				RequestLightCreationEvF::Ptr reqLightCreationEvPtr,
				RequestDestructionEvF::Ptr reqDestructionEvPtr)
			{
				reqGObjCreationEvF = std::make_unique<RequestGObjCreationEvF::Functor>(reqGObjCreationEvPtr);
				reqModelCreationEvF = std::make_unique<RequestModelCreationEvF::Functor>(reqModelCreationEvPtr);
				reqLightCreationEvF = std::make_unique<RequestLightCreationEvF::Functor>(reqLightCreationEvPtr);
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
			using ActivateF = Core::JSFunctorType<void, JObjectExplorer*, JUserPtr<JGameObject>, bool>;
		public:
			std::unique_ptr<ChangeParentF::Functor> changeParentF;
			std::unique_ptr<RenameF::Functor> renameF;
			std::unique_ptr<ActivateF::Functor> activateF;
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
			editorString->AddString(createGameObjectNode->GetNodeId(), { "Create GameObject", u8"���ӿ�����Ʈ ����" });
			editorString->AddString(createGameObjectNode->GetTooltipId(), { "Create selected gameObject's child in the scene", u8"������ ������ ���ӿ������� �ڽ��� �����մϴ�." });

			std::unique_ptr<JEditorPopupNode> createShapeNode =
				std::make_unique<JEditorPopupNode>("Shape", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, createGameObjectNode.get(), false);
			editorString->AddString(createShapeNode->GetNodeId(), { "Shape", u8"����" }); 

			std::unique_ptr<JEditorPopupNode> createCubeNode =
				std::make_unique<JEditorPopupNode>("Cube", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createCubeNode->GetNodeId(), { "Cube", u8"������ü" });

			std::unique_ptr<JEditorPopupNode> createGridNode =
				std::make_unique<JEditorPopupNode>("Grid", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createGridNode->GetNodeId(), { "Grid", u8"������" });

			std::unique_ptr<JEditorPopupNode> createCyilinderNode =
				std::make_unique<JEditorPopupNode>("Cyilinder", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createCyilinderNode->GetNodeId(), { "Cyilinder", u8"�����" });

			std::unique_ptr<JEditorPopupNode> createSphereNode =
				std::make_unique<JEditorPopupNode>("Sphere", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createSphereNode->GetNodeId(), { "Sphere", u8"��ü" });

			std::unique_ptr<JEditorPopupNode> createQuadNode =
				std::make_unique<JEditorPopupNode>("Quad", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createQuadNode->GetNodeId(), { "Quad", u8"�簢��" });

			std::unique_ptr<JEditorPopupNode> createEmptyNode =
				std::make_unique<JEditorPopupNode>("Empty", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createEmptyNode->GetNodeId(), { "Empty", u8"�� ������Ʈ" });

			std::unique_ptr<JEditorPopupNode> createLightNode =
				std::make_unique<JEditorPopupNode>("Light", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, createGameObjectNode.get(), false);
			editorString->AddString(createLightNode->GetNodeId(), { "Light", u8"����Ʈ" });

			std::unique_ptr<JEditorPopupNode> createDirectionalLightNode =
				std::make_unique<JEditorPopupNode>("Directional Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createDirectionalLightNode->GetNodeId(), { "Directional Light", u8"�𷺼ų� ����Ʈ" });

			std::unique_ptr<JEditorPopupNode> createPointLightNode =
				std::make_unique<JEditorPopupNode>("Point Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createPointLightNode->GetNodeId(), { "Point Light", u8"����Ʈ ����Ʈ" });

			std::unique_ptr<JEditorPopupNode> createSpotLightNode =
				std::make_unique<JEditorPopupNode>("Spot Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createSpotLightNode->GetNodeId(), { "Spot Light", u8"���� ����Ʈ" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), true);
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy JGameObject", u8"���ӿ�����Ʈ ����" });
			editorString->AddString(destroyNode->GetTooltipId(), { "Destroy selected gameObject in the scene", u8"������ ������ ���ӿ�����Ʈ�� �����մϴ�." });

			std::unique_ptr<JEditorPopupNode> renameNode =
				std::make_unique<JEditorPopupNode>("Rename JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), false);
			editorString->AddString(renameNode->GetNodeId(), { "Rename", u8"���̸�" });

			using RequestGObjCreationEvF = JObjectExplorerCreationImpl::RequestGObjCreationEvF;
			using RequestLightCreationEvF = JObjectExplorerCreationImpl::RequestLightCreationEvF;
			using RequestDestructionEvF = JObjectExplorerCreationImpl::RequestDestructionEvF;
			using RenameF = JObjectExplorerSettingImpl::RenameF;

			//register popup callback
			//enable = condition
			//select = doSomething
			createCubeNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::CUBE));
			createGridNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::GRID));
			createCyilinderNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::CYILINDER));
			createSphereNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::SPHERE));
			createQuadNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::QUAD));
			createEmptyNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creationImpl->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::EMPTY));

			createDirectionalLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creationImpl->reqLightCreationEvF, this, J_LIGHT_TYPE::DIRECTIONAL));
			createPointLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creationImpl->reqLightCreationEvF, this, J_LIGHT_TYPE::POINT));
			createSpotLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creationImpl->reqLightCreationEvF, this, J_LIGHT_TYPE::SPOT));

			destroyNode->RegisterSelectBind(std::make_unique<RequestDestructionEvF::CompletelyBind>(*creationImpl->reqDestructionEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));
			renameNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*settingImpl->renameF, this));
			renameNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));

			explorerPopup = std::make_unique<JEditorPopupMenu>("explorerPopup", std::move(explorerPopupRootNode));
			explorerPopup->AddPopupNode(std::move(createGameObjectNode));
			explorerPopup->AddPopupNode(std::move(createShapeNode));
			explorerPopup->AddPopupNode(std::move(createCubeNode));
			explorerPopup->AddPopupNode(std::move(createGridNode));
			explorerPopup->AddPopupNode(std::move(createCyilinderNode));
			explorerPopup->AddPopupNode(std::move(createSphereNode));
			explorerPopup->AddPopupNode(std::move(createQuadNode));
			explorerPopup->AddPopupNode(std::move(createEmptyNode));
			explorerPopup->AddPopupNode(std::move(createLightNode));
			explorerPopup->AddPopupNode(std::move(createDirectionalLightNode));
			explorerPopup->AddPopupNode(std::move(createPointLightNode));
			explorerPopup->AddPopupNode(std::move(createSpotLightNode));
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

			//register request
			//creation request use creation interface
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
			auto requestCreateLightLam = [](JObjectExplorer* explorer, J_LIGHT_TYPE lightType)
			{
				if (!explorer->root.IsValid())
					return;

				JUserPtr<JGameObject> parent = Core::ConvertChildUserPtr<JGameObject>(explorer->GetHoveredObject());
				if (parent == nullptr)
					parent = explorer->root;

				JEditorCreationHint creationHint = JEditorCreationHint(explorer,
					false, true, false, true,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(parent->GetOwnerScene()),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, explorer->GetClearTaskFunctor());

				JObjectExplorerCreationImpl* impl = explorer->creationImpl.get();
				impl->light.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint, parent->GetGuid(), std::move(lightType));
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
			creationImpl = std::make_unique<JObjectExplorerCreationImpl>(requestCreateGObjLam, requestCreateModelLam, requestCreateLightLam, requestDestroyLam);

			//register can create & create
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
			
			auto canCreateLightLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const J_LIGHT_TYPE lightType)
			{
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (ownerPtr == nullptr)
					return false;

				auto rawPtr = Core::GetRawPtr(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				if (ownerPtr->GetTypeInfo().IsChildOf<JScene>() && rawPtr != nullptr)
					return static_cast<JGameObject*>(rawPtr)->GetOwnerScene()->GetGuid() == ownerPtr->GetGuid();
				else
					return false;
			};
			auto createLightLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t parentGuid, const J_LIGHT_TYPE lightType)
			{ 
				auto parentUserPtr = Core::GetUserPtr<JGameObject>(Core::JTypeInstanceSearchHint(JGameObject::StaticTypeInfo(), parentGuid));
				JGCI::CreateLight(parentUserPtr, guid, OBJECT_FLAG_NONE, lightType);
			};

			creationImpl->gameObject.GetCreationInterface()->RegisterCanCreationF(canCreationGobjLam);
			creationImpl->gameObject.GetCreationInterface()->RegisterObjectCreationF(creationGobjLam);
			creationImpl->model.GetCreationInterface()->RegisterCanCreationF(canCreationModelLam);
			creationImpl->model.GetCreationInterface()->RegisterObjectCreationF(creationModelLam);
			creationImpl->light.GetCreationInterface()->RegisterCanCreationF(canCreateLightLam);
			creationImpl->light.GetCreationInterface()->RegisterObjectCreationF(createLightLam);
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
			auto activateLam = [](JObjectExplorer* objEx, JUserPtr<JGameObject> obj, bool value)
			{
				if (value)
					JGameObjectPrivate::ActivateInterface::Activate(obj);
				else
					JGameObjectPrivate::ActivateInterface::DeActivate(obj);
			};
			settingImpl = std::make_unique<JObjectExplorerSettingImpl>();
			settingImpl->changeParentF = std::make_unique<JObjectExplorerSettingImpl::ChangeParentF::Functor>(changeParentLam);
			settingImpl->renameF = std::make_unique<JObjectExplorerSettingImpl::RenameF::Functor>(renameLam);
			settingImpl->activateF = std::make_unique<JObjectExplorerSettingImpl::ActivateF::Functor>(activateLam);
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
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
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
			JGui::SameLine();
		}
		void JObjectExplorer::ObjectExplorerOnScreen(const JUserPtr<JGameObject>& gObj, const bool isAcivatedSearch)
		{
			const std::string objName = JCUtil::WstrToU8Str(gObj->GetName());
			const size_t objGuid = gObj->GetGuid();

			//JGuiTreeNodeFlags_Selected
			bool isNodeOpen = false;
			const bool isRenameActivaetd = renameHelper->IsActivated() && renameHelper->IsRenameTar(objGuid);

			//selected mark�� �̿��ϸ� �ڽ� gameObject�� �����ϴ� node���� mark�Ǿ�����Ƿ�
			//�̴� Explorer���� ����ϴ� �ٰ� �ƴϹǷ� ���� Ŭ���� gameObject�� ����Ǵ�
			//SelectedMap�� �����ؼ� mark
			const bool isSelected = IsSelectedObject(objGuid);
			const bool isActivated = gObj->IsActivated();

			const J_GUI_TREE_NODE_FLAG_ baseFlags = J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW |
				J_GUI_TREE_NODE_FLAG_EXTEND_HIT_BOX_WIDTH |
				J_GUI_TREE_NODE_FLAG_FRAMED;

			//if(!isSelected && gObj->IsSelectedbyEditor())
			//	RequestPushSelectObject(Core::GetUserPtr(gObj));
			 
			const bool canOnScreen = searchBarHelper->CanSrcNameOnScreen(objName);
			if (canOnScreen)
			{
				if (isRenameActivaetd)
				{
					//can't select and hover
					//fixed hover object when start rename ev
					PushTreeNodeColorSet(isActivated, isSelected); 
					isNodeOpen = JGui::IsTreeNodeOpend(JGui::CreateGuiLabel(objName, objGuid, GetName() + "TreeNode"), baseFlags);
					PopTreeNodeColorSet(isActivated, isSelected);
					DisplayActSignalWidget(gObj, false);
					renameHelper->Update(isNodeOpen);
				}
				else
				{ 
					if (isAcivatedSearch)
						JGui::SetNextItemOpen(true);		
					PushTreeNodeColorSet(isActivated, isSelected);
					isNodeOpen = JGui::TreeNodeEx(JGui::CreateGuiLabel(objName, objGuid, GetName() + "TreeNode"), baseFlags);
					PopTreeNodeColorSet(isActivated, isSelected);
					if (JGui::IsLastItemHovered())
						SetHoveredObject(gObj);

					DisplayActSignalWidget(gObj, true);
					if (JGui::IsLastItemAnyClicked(false) && !JGui::IsKeyDown(Core::J_KEYCODE::CONTROL))
					{
						RequestPushSelectObject(gObj);
						SetContentsClick(true);
					}
					if (isNodeOpen)
					{
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
					JGui::UnIndent();
				else if (isNodeOpen)
					JGui::TreePop();
			}
		}
		void JObjectExplorer::DisplayActSignalWidget(const JUserPtr<JGameObject>& gObj, const bool allowDisplaySeletable)
		{   
			const bool isActivated = gObj->IsActivated();
			const float radius = JGui::GetLastItemRectSize().y * 0.25f;
			const float xOffset = radius * 2;
			const JVector2<uint> centerPos = JVector2<uint>(JGui::GetLastItemRectMax().x - xOffset, JGui::GetLastItemRectMin().y + JGui::GetLastItemRectSize().y * 0.5f);
			JVector4 color = isActivated ? actColor : deActColor;

			const bool isHover = allowDisplaySeletable && JGui::IsMouseInRect(centerPos - JVector2<uint>(radius, radius), JVector2<uint>(radius * 2, radius * 2));
			if (isHover)
				color += GetSelectableColorFactor(false, isHover);

			JGui::DrawCircleFilledColor(centerPos, radius, color, true);
			if (gObj != nullptr)
			{
				if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT) && isHover)
				{
					using ActivateF = JObjectExplorerSettingImpl::ActivateF;

					std::string taskType = !isActivated ? "Activate" : "DeActivate";
					std::string taskName = taskType + " " + JCUtil::WstrToU8Str(gObj->GetName());
					std::string taskDesc = "";

					auto doBind = std::make_unique<ActivateF::CompletelyBind>(*settingImpl->activateF, this, JUserPtr<JGameObject>(gObj), !gObj->IsActivated());
					auto undoBind = std::make_unique<ActivateF::CompletelyBind>(*settingImpl->activateF, this, JUserPtr<JGameObject>(gObj), gObj->IsActivated());
					auto evStruct = std::make_unique<JEditorTSetBindFuncEvStruct>(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind));

					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
				}  
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
			RemoveListener(*JEditorEvent::EvInterface(), GetGuid());
			JEditorWindow::DoDeActivate();
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
			if (!eventStruct->CanExecuteOtherEv(senderGuid, GetGuid()))
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				explorerPopup->SetOpen(false);
		}
	}
}