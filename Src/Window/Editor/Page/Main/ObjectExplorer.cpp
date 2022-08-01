#include"ObjectExplorer.h"
#include"../EditorAttribute.h"
#include"../../String/EditorString.h"
#include"../../Popup/EditorPopup.h"
#include"../../Popup/EditorPopupNode.h" 
#include"../../Utility/EditorUtility.h"  
#include"../../Event/EditorEventStruct.h"
#include"../../../../Utility/JCommonUtility.h"   
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/GameObject/JGameObjectFactory.h"
#include"../../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/Model/JModel.h"
#include"../../../../../Lib/imgui/imgui.h"  
 
namespace JinEngine
{
	ObjectExplorer::ObjectExplorer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{
		//EditorString Init
		editorString = std::make_unique<EditorString>();

		//PopupInit
		std::unique_ptr<EditorPopupNode> explorerPopupRootNode =
			std::make_unique<EditorPopupNode>("ObjectExplorer Popup Root", EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

		//std::unique_ptr<EditorPopupNode> renameGameObjectNode =
		//	std::make_unique<EditorPopupNode>("Rename JGameObject", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, explorerPopupRootNode.get(), false);
		//editorString->AddString(renameGameObjectNode->GetNodeId(), {"Rename a JGameObject", u8"새이름" }); 

		std::unique_ptr<EditorPopupNode> createGameObjectNode =
			std::make_unique<EditorPopupNode>("Create JGameObject", EDITOR_POPUP_NODE_TYPE::INTERNAL, explorerPopupRootNode.get(), true);
		editorString->AddString(createGameObjectNode->GetNodeId(), { "Create JGameObject", u8"게임오브젝트 생성" });
		editorString->AddString(createGameObjectNode->GetTooltipId(), { "Create selected gameObject's child in the scene", u8"씬내에 선택한 게임오브젝에 자식을 생성합니다." });

		std::unique_ptr<EditorPopupNode> createCubeNode =
			std::make_unique<EditorPopupNode>("Cube", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createCubeNode->GetNodeId(), { "Cube", u8"정육면체" });

		std::unique_ptr<EditorPopupNode> createGridNode =
			std::make_unique<EditorPopupNode>("Grid", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createGridNode->GetNodeId(), { "Grid", u8"격자판" });

		std::unique_ptr<EditorPopupNode> createCyilinderNode =
			std::make_unique<EditorPopupNode>("Cyilinder", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createCyilinderNode->GetNodeId(), { "Cyilinder", u8"원기둥" });

		std::unique_ptr<EditorPopupNode> createSphereNode =
			std::make_unique<EditorPopupNode>("Sphere", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createSphereNode->GetNodeId(), { "Sphere", u8"구체" });

		std::unique_ptr<EditorPopupNode> createQuadNode =
			std::make_unique<EditorPopupNode>("Quad", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createQuadNode->GetNodeId(), { "Quad", u8"사각형" });

		std::unique_ptr<EditorPopupNode> createEmptyNode =
			std::make_unique<EditorPopupNode>("Empty", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createGameObjectNode.get());
		editorString->AddString(createEmptyNode->GetNodeId(), { "Empty", u8"빈 오브젝트" });

		std::unique_ptr<EditorPopupNode> eraseGameObjectNode =
			std::make_unique<EditorPopupNode>("Erase JGameObject", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, explorerPopupRootNode.get(), true);
		editorString->AddString(eraseGameObjectNode->GetNodeId(), { "Erase JGameObject", u8"게임오브젝트 삭제" });
		editorString->AddString(eraseGameObjectNode->GetTooltipId(), { "Erase selected gameObject in the scene", u8"씬내에 선택한 게임오브젝트를 삭제합니다." });

		JDelegate<void(EditorUtility*)> renameFunc;
		renameFunc.connect([=](EditorUtility* util) {  });

		JDelegate<void(EditorUtility*)> createCubeFunc;
		createCubeFunc.connect([=](EditorUtility* util) { JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE); });

		JDelegate<void(EditorUtility*)> createGridFunc;
		createGridFunc.connect([=](EditorUtility* util) { JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID); });

		JDelegate<void(EditorUtility*)> createCyilinderFunc;
		createCyilinderFunc.connect([=](EditorUtility* util) {JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER); });

		JDelegate<void(EditorUtility*)> createSphereFunc;
		createSphereFunc.connect([=](EditorUtility* util) {JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE); });

		JDelegate<void(EditorUtility*)> createQuadFunc;
		createQuadFunc.connect([=](EditorUtility* util) {JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD); });

		JDelegate<void(EditorUtility*)> createEmptyFunc;
		createEmptyFunc.connect([=](EditorUtility* util) {JGFU::CreateShape(*(static_cast<JGameObject*>(util->selectedObject)), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY); });

		JDelegate<void(EditorUtility*)> eraseGameObjectFunc;
		eraseGameObjectFunc.connect([=](EditorUtility* editorUtility)
			{				
				JGameObject* tarObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
				std::unique_ptr<EditorDeSelectGameObjectEvStruct> selectEvStruct = std::make_unique<EditorDeSelectGameObjectEvStruct>(tarObj);
				this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::DESELECT_GAMEOBJECT, selectEvStruct.get());
				JGameObject::EraseGameObject(tarObj);
			});
		gameObjFunc.emplace(createCubeNode->GetNodeId(), createCubeFunc);
		gameObjFunc.emplace(createGridNode->GetNodeId(), createGridFunc);
		gameObjFunc.emplace(createCyilinderNode->GetNodeId(), createCyilinderFunc);
		gameObjFunc.emplace(createSphereNode->GetNodeId(), createSphereFunc);
		gameObjFunc.emplace(createQuadNode->GetNodeId(), createQuadFunc);
		gameObjFunc.emplace(createEmptyNode->GetNodeId(), createEmptyFunc);
		//eraseGameObjFunc.emplace(eraseGameObjectNode->GetNodeId(), eraseGameObjectFunc);

		explorerPopup = std::make_unique<EditorPopup>("explorerPopup", std::move(explorerPopupRootNode));
		explorerPopup->AddPopupNode(std::move(createGameObjectNode));
		explorerPopup->AddPopupNode(std::move(createCubeNode));
		explorerPopup->AddPopupNode(std::move(createGridNode));
		explorerPopup->AddPopupNode(std::move(createCyilinderNode));
		explorerPopup->AddPopupNode(std::move(createSphereNode));
		explorerPopup->AddPopupNode(std::move(createQuadNode));
		explorerPopup->AddPopupNode(std::move(createEmptyNode));
		explorerPopup->AddPopupNode(std::move(eraseGameObjectNode));
	}
	ObjectExplorer::~ObjectExplorer()
	{

	}
	void ObjectExplorer::Initialize(EditorUtility* editorUtility) noexcept
	{
		editorUtility->selectedObject = JResourceManager::Instance().GetMainScene()->GetRootGameObject();
	}
	void ObjectExplorer::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		BuildObjectExplorer(JResourceManager::Instance().GetMainScene(), editorUtility);
	}
	bool ObjectExplorer::Activate(EditorUtility* editorUtility)
	{
		if (Editor::Activate(editorUtility))
		{
			this->AddEventListener(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::MOUSE_CLICK);
			return true;
		}
		else
			return false;
	}
	bool ObjectExplorer::DeActivate(EditorUtility* editorUtility)
	{
		if (Editor::DeActivate(editorUtility))
		{
			this->EraseListener(*editorUtility->EvInterface(), GetGuid()); 
			return true;
		}
		else
			return false;
	}
	bool ObjectExplorer::OnFocus(EditorUtility* editorUtility)
	{
		if (Editor::OnFocus(editorUtility))
			return true;
		else
			return false;
	}
	bool ObjectExplorer::OffFocus(EditorUtility* editorUtility)
	{
		if (Editor::OffFocus(editorUtility))
		{
			explorerPopup->SetOpen(false);
			return true;
		}
		else
			return false;
	}
	void ObjectExplorer::BuildObjectExplorer(JScene* scene, EditorUtility* editorUtility)
	{
		ObjectExplorerEvent objectExplorerEvent;
		ObjectExplorerOnScreen(scene, scene->GetRootGameObject(), editorUtility, objectExplorerEvent);
		SetTreeNodeDefaultColor();
 
		if (objectExplorerEvent.selectGameObj != nullptr && objectExplorerEvent.tarObject != nullptr)
		{
			if (objectExplorerEvent.eventType == OBJECT_EXPLORER_EVENT_TYPE::CHANGE_PARENT)
			{
				JGameObject* tarGameObj = dynamic_cast<JGameObject*>(objectExplorerEvent.tarObject);
				tarGameObj->ChangeParent(objectExplorerEvent.selectGameObj);
			}
			else if (objectExplorerEvent.eventType == OBJECT_EXPLORER_EVENT_TYPE::MAKE_MODEL)
			{
				JModel* tarModel = dynamic_cast<JModel*>(objectExplorerEvent.tarObject);
				if (tarModel != nullptr)
					objectExplorerEvent.selectGameObj = JGFI::Copy(*tarModel->GetModelRoot(), *objectExplorerEvent.selectGameObj);
			}
		}

		if (explorerPopup->IsOpen())
		{
			EDITOR_POPUP_NODE_RES res;
			size_t menuGuid;	 
			explorerPopup->ExecutePopup(editorString.get(), res, menuGuid); 
			if (res == EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
			{
				auto func = gameObjFunc.find(menuGuid);
				if (func != gameObjFunc.end())
				{
					if (editorUtility->selectedObject != nullptr && 
						editorUtility->selectedObject->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
						func->second.Invoke(editorUtility);
				}
				explorerPopup->SetOpen(false);
			}
		}
		 
		if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
		{
			if (editorUtility->rightMouseClick)
				explorerPopup->SetOpen(!explorerPopup->IsOpen());
			else if (editorUtility->leftMouseClick && explorerPopup->IsOpen() && !explorerPopup->IsMouseInPopup())
				explorerPopup->SetOpen(false);
		} 
		ImGui::SameLine();
	}
	void ObjectExplorer::ObjectExplorerOnScreen(JScene* scene, JGameObject* nowGameObj, EditorUtility* editorUtility, ObjectExplorerEvent& objectExplorerEvent)
	{
		//ImGuiTreeNodeFlags_Selected
		bool isSelected = false;
		std::string name = nowGameObj->GetName(); 
		if (editorUtility->selectedObject != nullptr && editorUtility->selectedObject->GetGuid() == nowGameObj->GetGuid())
		{
			isSelected = true;
			SetTreeNodeSelectColor();
		}
		 
		if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if(isSelected)
				SetTreeNodeDefaultColor();

			if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
			{
				editorUtility->selectedObject = nowGameObj;
				std::unique_ptr<EditorSelectGameObjectEvStruct> selectEvStruct =
					std::make_unique<EditorSelectGameObjectEvStruct>(nowGameObj);

				this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::SELECT_GAMEOBJECT, selectEvStruct.get());
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				int noUse = 0;
				editorUtility->isDrag = true;
				editorUtility->selectedObject = nowGameObj;
				ImGui::Text(name.c_str());
				ImGui::SetDragDropPayload(name.c_str(), nowGameObj, sizeof(JObject*));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (editorUtility->selectedObject != nullptr && editorUtility->isDrag)
				{
					const std::string itemName = editorUtility->selectedObject->GetName();
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(itemName.c_str(), ImGuiDragDropFlags_None);
					const J_OBJECT_TYPE objType = editorUtility->selectedObject->GetObjectType();

					if (objType == J_OBJECT_TYPE::GAME_OBJECT && payload)
					{
						objectExplorerEvent.eventType = OBJECT_EXPLORER_EVENT_TYPE::CHANGE_PARENT;
						objectExplorerEvent.selectGameObj = nowGameObj;
						objectExplorerEvent.tarObject = editorUtility->selectedObject;
						editorUtility->isDrag = false;
					}
					else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT && payload)
					{
						JResourceObject* selectResource = dynamic_cast<JResourceObject*>(editorUtility->selectedObject);
						const J_RESOURCE_TYPE resourceType = selectResource->GetResourceType();
						if (resourceType == J_RESOURCE_TYPE::MODEL)
						{
							objectExplorerEvent.eventType = OBJECT_EXPLORER_EVENT_TYPE::MAKE_MODEL;
							objectExplorerEvent.selectGameObj = nowGameObj;
							objectExplorerEvent.tarObject = editorUtility->selectedObject;
						}
						editorUtility->isDrag = false;
					}
				}
				ImGui::EndDragDropTarget();
			}

			const uint childrenCount = nowGameObj->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
			{
				JGameObject* child = nowGameObj->GetChild(i);
				if ((child->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
					continue;

				ObjectExplorerOnScreen(scene, child, editorUtility, objectExplorerEvent);
			}
			ImGui::TreePop();
		}
	}
	void ObjectExplorer::OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)
	{
		if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
			return;
		if (eventType == EDITOR_EVENT::MOUSE_CLICK)
			explorerPopup->SetOpen(false);
	}
}
