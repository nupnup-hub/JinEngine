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
#include"../../../EditTool/JEditorSearchBar.h"
#include"../../../EditTool/JEditorTreeStructure.h"
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

		class JObjectExplorerCreationFunctor
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
			JObjectExplorerCreationFunctor(RequestGObjCreationEvF::Ptr reqGObjCreationEvPtr,
				RequestModelCreationEvF::Ptr reqModelCreationEvPtr,
				RequestLightCreationEvF::Ptr reqLightCreationEvPtr,
				RequestDestructionEvF::Ptr reqDestructionEvPtr)
			{
				reqGObjCreationEvF = std::make_unique<RequestGObjCreationEvF::Functor>(reqGObjCreationEvPtr);
				reqModelCreationEvF = std::make_unique<RequestModelCreationEvF::Functor>(reqModelCreationEvPtr);
				reqLightCreationEvF = std::make_unique<RequestLightCreationEvF::Functor>(reqLightCreationEvPtr);
				reqDestructionEvF = std::make_unique<RequestDestructionEvF::Functor>(reqDestructionEvPtr);
			}
			~JObjectExplorerCreationFunctor()
			{
				dS.Clear();
			}
		};

		class JObjectExplorerSettingFunctor
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
			const J_EDITOR_WINDOW_FLAG windowFlag,
			const std::vector<size_t>& listenWindowGuidVec)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>();
			renameHelper = std::make_unique<JEditorRenameHelper>();
			searchBarHelper = std::make_unique<JEditorSearchBar>(false);
			treeStrcture = std::make_unique<JEditorTreeStructure>();

			for (const auto& data : listenWindowGuidVec)
				PushOtherWindowGuidForListenEv(data);
			InitializeCreationImpl();
			InitializeSettingImpl();

			//PopupInit
			std::unique_ptr<JEditorPopupNode> explorerPopupRootNode =
				std::make_unique<JEditorPopupNode>("JObjectExplorer Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createGameObjectNode =
				std::make_unique<JEditorPopupNode>("Create GameObject", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, explorerPopupRootNode.get(), true);
			editorString->AddString(createGameObjectNode->GetNodeId(), { "Create GameObject", u8"게임오브젝트 생성" });
			editorString->AddString(createGameObjectNode->GetTooltipId(), { "Create selected gameObject's child in the scene", u8"씬내에 선택한 게임오브젝에 자식을 생성합니다." });

			std::unique_ptr<JEditorPopupNode> createShapeNode =
				std::make_unique<JEditorPopupNode>("Shape", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, createGameObjectNode.get(), false);
			editorString->AddString(createShapeNode->GetNodeId(), { "Shape", u8"도형" }); 

			std::unique_ptr<JEditorPopupNode> createCubeNode =
				std::make_unique<JEditorPopupNode>("Cube", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createCubeNode->GetNodeId(), { "Cube", u8"정육면체" });

			std::unique_ptr<JEditorPopupNode> createGridNode =
				std::make_unique<JEditorPopupNode>("Grid", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createGridNode->GetNodeId(), { "Grid", u8"격자판" });

			std::unique_ptr<JEditorPopupNode> createCyilinderNode =
				std::make_unique<JEditorPopupNode>("Cyilinder", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createCyilinderNode->GetNodeId(), { "Cyilinder", u8"원기둥" });

			std::unique_ptr<JEditorPopupNode> createSphereNode =
				std::make_unique<JEditorPopupNode>("Sphere", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createSphereNode->GetNodeId(), { "Sphere", u8"구체" });

			std::unique_ptr<JEditorPopupNode> createQuadNode =
				std::make_unique<JEditorPopupNode>("Quad", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createQuadNode->GetNodeId(), { "Quad", u8"사각형" });

			std::unique_ptr<JEditorPopupNode> createEmptyNode =
				std::make_unique<JEditorPopupNode>("Empty", J_EDITOR_POPUP_NODE_TYPE::LEAF, createShapeNode.get());
			editorString->AddString(createEmptyNode->GetNodeId(), { "Empty", u8"빈 오브젝트" });

			std::unique_ptr<JEditorPopupNode> createLightNode =
				std::make_unique<JEditorPopupNode>("Light", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, createGameObjectNode.get(), false);
			editorString->AddString(createLightNode->GetNodeId(), { "Light", u8"라이트" });

			std::unique_ptr<JEditorPopupNode> createDirectionalLightNode =
				std::make_unique<JEditorPopupNode>("Directional Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createDirectionalLightNode->GetNodeId(), { "Directional Light", u8"디렉셔널 라이트" });

			std::unique_ptr<JEditorPopupNode> createPointLightNode =
				std::make_unique<JEditorPopupNode>("Point Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createPointLightNode->GetNodeId(), { "Point Light", u8"포인트 라이트" });

			std::unique_ptr<JEditorPopupNode> createSpotLightNode =
				std::make_unique<JEditorPopupNode>("Spot Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createSpotLightNode->GetNodeId(), { "Spot Light", u8"스팟 라이트" });

			std::unique_ptr<JEditorPopupNode> createRectLightNode =
				std::make_unique<JEditorPopupNode>("Spot Light", J_EDITOR_POPUP_NODE_TYPE::LEAF, createLightNode.get());
			editorString->AddString(createRectLightNode->GetNodeId(), { "Rect Light", u8"사각 라이트" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), true);
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy JGameObject", u8"게임오브젝트 삭제" });
			editorString->AddString(destroyNode->GetTooltipId(), { "Destroy selected gameObject in the scene", u8"씬내에 선택한 게임오브젝트를 삭제합니다." });

			std::unique_ptr<JEditorPopupNode> renameNode =
				std::make_unique<JEditorPopupNode>("Rename JGameObject", J_EDITOR_POPUP_NODE_TYPE::LEAF, explorerPopupRootNode.get(), false);
			editorString->AddString(renameNode->GetNodeId(), { "Rename", u8"새이름" });

			using RequestGObjCreationEvF = JObjectExplorerCreationFunctor::RequestGObjCreationEvF;
			using RequestLightCreationEvF = JObjectExplorerCreationFunctor::RequestLightCreationEvF;
			using RequestDestructionEvF = JObjectExplorerCreationFunctor::RequestDestructionEvF;
			using RenameF = JObjectExplorerSettingFunctor::RenameF;

			//register popup callback
			//enable = condition
			//select = doSomething
			createCubeNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::CUBE));
			createGridNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::GRID));
			createCyilinderNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::CYILINDER));
			createSphereNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::SPHERE));
			createQuadNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::QUAD));
			createEmptyNode->RegisterSelectBind(std::make_unique<RequestGObjCreationEvF::CompletelyBind>(*creation->reqGObjCreationEvF, this, J_DEFAULT_SHAPE::EMPTY));

			createDirectionalLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creation->reqLightCreationEvF, this, J_LIGHT_TYPE::DIRECTIONAL));
			createPointLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creation->reqLightCreationEvF, this, J_LIGHT_TYPE::POINT));
			createSpotLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creation->reqLightCreationEvF, this, J_LIGHT_TYPE::SPOT));
			createRectLightNode->RegisterSelectBind(std::make_unique<RequestLightCreationEvF::CompletelyBind>(*creation->reqLightCreationEvF, this, J_LIGHT_TYPE::RECT));

			destroyNode->RegisterSelectBind(std::make_unique<RequestDestructionEvF::CompletelyBind>(*creation->reqDestructionEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));
			renameNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*setting->renameF, this));
			renameNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));

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
			explorerPopup->AddPopupNode(std::move(createRectLightNode));
			explorerPopup->AddPopupNode(std::move(destroyNode));
			explorerPopup->AddPopupNode(std::move(renameNode));
		}
		JObjectExplorer::~JObjectExplorer()
		{
			creation.reset();
			setting.reset();
		}
		void JObjectExplorer::InitializeCreationImpl()
		{
			if (creation != nullptr)
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);
				 
				JObjectExplorerCreationFunctor* impl = explorer->creation.get();
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JObjectExplorerCreationFunctor* impl = explorer->creation.get();
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JObjectExplorerCreationFunctor* impl = explorer->creation.get();
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JObjectExplorerCreationFunctor* impl = explorer->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creation = std::make_unique<JObjectExplorerCreationFunctor>(requestCreateGObjLam, requestCreateModelLam, requestCreateLightLam, requestDestroyLam);

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

			creation->gameObject.GetCreationInterface()->RegisterCanCreationF(canCreationGobjLam);
			creation->gameObject.GetCreationInterface()->RegisterObjectCreationF(creationGobjLam);
			creation->model.GetCreationInterface()->RegisterCanCreationF(canCreationModelLam);
			creation->model.GetCreationInterface()->RegisterObjectCreationF(creationModelLam);
			creation->light.GetCreationInterface()->RegisterCanCreationF(canCreateLightLam);
			creation->light.GetCreationInterface()->RegisterObjectCreationF(createLightLam);
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
			setting = std::make_unique<JObjectExplorerSettingFunctor>();
			setting->changeParentF = std::make_unique<JObjectExplorerSettingFunctor::ChangeParentF::Functor>(changeParentLam);
			setting->renameF = std::make_unique<JObjectExplorerSettingFunctor::RenameF::Functor>(renameLam);
			setting->activateF = std::make_unique<JObjectExplorerSettingFunctor::ActivateF::Functor>(activateLam);
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
				isFocusedThisFrame = JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW);
				UpdateMouseClick();
				searchBarHelper->UpdateSearchBar();			 
				treeStrcture->Begin();
				BuildObjectExplorer();
				treeStrcture->End();   
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

			//selected mark를 이용하면 자식 gameObject를 참조하는 node까지 mark되어버리므로
			//이는 Explorer에서 기대하는 바가 아니므로 직접 클릭한 gameObject만 저장되는
			//SelectedMap을 참조해서 mark
			const bool isSelected = IsSelectedObject(objGuid);
			const bool isActivated = gObj->IsActivated();

			//if(!isSelected && gObj->IsSelectedbyEditor())
			//	RequestPushSelectObject(Core::GetUserPtr(gObj));
			  
			const bool canOnScreen = searchBarHelper->CanSrcNameOnScreen(objName);
			if (canOnScreen)
			{
				if (isRenameActivaetd)
				{
					//can't select and hover
					//fixed hover object when start rename ev
					isNodeOpen = treeStrcture->CheckTreeNodeIsOpen(JGui::CreateGuiLabel(objName, objGuid, GetName() + "TreeNode"), treeStrcture->GetBaseFlag(), IsFocus(), isActivated, isSelected);
					DisplayActSignalWidget(gObj, false);
					renameHelper->Update(isNodeOpen);
				}
				else
				{ 
					if (isAcivatedSearch)
						JGui::SetNextItemOpen(true);	
					isNodeOpen = treeStrcture->DisplayTreeNode(JGui::CreateGuiLabel(objName, objGuid, GetName() + "TreeNode"), treeStrcture->GetBaseFlag(), IsFocus(), isActivated, isSelected);
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
						auto dragResult = TryGetDraggingTarget();
						for (const auto& data : dragResult)
						{
							if (!data.IsValid())
								continue;

							Core::JTypeInfo& typeInfo = data->GetTypeInfo();
							JObject* obj = typeInfo.IsChildOf<JObject>() ? static_cast<JObject*>(data.Get()) : nullptr;
							if (obj != nullptr && obj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
							{
								JUserPtr<JGameObject> selectedObj;
								selectedObj.ConnnectChild(data);

								using ChangeParentF = JObjectExplorerSettingFunctor::ChangeParentF;

								std::string taskName = "Change parent";
								std::string taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " + selectedObj->GetName() + L" to " + gObj->GetName());
									
								auto doBind = std::make_unique<ChangeParentF::CompletelyBind>(*setting->changeParentF, this, JUserPtr<JGameObject>(selectedObj), JUserPtr<JGameObject>(gObj));
								auto undoBind = std::make_unique<ChangeParentF::CompletelyBind>(*setting->changeParentF, this, JUserPtr<JGameObject>(selectedObj), selectedObj->GetParent());
								auto evStruct = std::make_unique<JEditorTSetBindFuncEvStruct>(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind));

								AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
							}
							else if (obj != nullptr && obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
							{
								JUserPtr<JMeshGeometry> sMesh;
								sMesh.ConnnectChild(data);
								if (sMesh.IsValid())
									creation->reqModelCreationEvF->Invoke(this, gObj, sMesh);
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
				color += JGui::GetSelectableColorFactor(IsFocus(), false, isHover);

			JGui::DrawCircleFilledColor(centerPos, radius, color, true);
			if (gObj != nullptr)
			{
				if (isFocusedThisFrame && JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT) && isHover)
				{
					using ActivateF = JObjectExplorerSettingFunctor::ActivateF;

					std::string taskType = !isActivated ? "Activate" : "DeActivate";
					std::string taskName = taskType + " " + JCUtil::WstrToU8Str(gObj->GetName());
					std::string taskDesc = "";

					auto doBind = std::make_unique<ActivateF::CompletelyBind>(*setting->activateF, this, JUserPtr<JGameObject>(gObj), !gObj->IsActivated());
					auto undoBind = std::make_unique<ActivateF::CompletelyBind>(*setting->activateF, this, JUserPtr<JGameObject>(gObj), gObj->IsActivated());
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
		void JObjectExplorer::LoadEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::LoadEditorWindow(tool);
			treeStrcture->LoadData(tool);
		}
		void JObjectExplorer::StoreEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::StoreEditorWindow(tool);
			JGuiWindowInfo info;
			JGui::GetWindowInfo(GetName(), info);
			treeStrcture->StoreData(tool, info.windowID);
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