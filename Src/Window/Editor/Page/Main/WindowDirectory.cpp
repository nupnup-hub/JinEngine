#include"WindowDirectory.h"
#include"../EditorAttribute.h"

#include"../../String/EditorString.h"
#include"../../Popup/EditorPopup.h"
#include"../../Popup/EditorPopupNode.h"
#include"../../Utility/EditorUtility.h" 
#include"../../Event/EditorEventStruct.h"
#include"../../Utility/EditorObjectPositionCalculator.h"
#include"../../../../Utility/JCommonUtility.h"   
#include"../../../../Utility/JMathHelper.h"   
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/Material/JMaterial.h"
#include"../../../../Object/Resource/JResourceObjectFactory.h"
#include"../../../../Object/Resource/Scene/Preview/PreviewScene.h"
#include"../../../../Object/Resource/Scene/Preview/PreviewSceneGroup.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Graphic/JGraphic.h" 
#include"../../../../Graphic/JGraphicResourceManager.h"
#include"../../../../Application/JApplicationVariable.h"  
#include"../../../../../Lib/imgui/imgui.h"

using namespace std;
namespace JinEngine
{
	class JAnimationController;
	class JMaterial;

	static int DirectoryLevelFinder(const std::string& path, std::wstring& copyTarget)
	{
		std::wstring copy = JCommonUtility::U8StringToWstring(path);
		int st = 0;
		size_t index = 0;
		int level = 0;
		while ((index = copy.find_first_of(L"\\")) != std::wstring::npos)
		{
			level++;
			copy.erase(st, index + 1);
		}
		copyTarget = copy;
		return level;
	}
	WindowDirectory::WindowDirectory(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid), previewGuid(JCommonUtility::CalculateGuid(GetName() + "PreviewGroup"))
	{
		//EditorString Init
		editorString = std::make_unique<EditorString>();
		selectorIconSlidebarId = JCommonUtility::CalculateGuid("Icon Size");
		editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기 조절" });

		//Popup Init
		std::unique_ptr<EditorPopupNode> fileViewPopupRootNode =
			std::make_unique<EditorPopupNode>("Window JDirectory FileView Popup Root", EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);
		
		std::unique_ptr<EditorPopupNode> createResourceNode =
			std::make_unique<EditorPopupNode>("CreateResource", EDITOR_POPUP_NODE_TYPE::INTERNAL, fileViewPopupRootNode.get());
		editorString->AddString(createResourceNode->GetNodeId(), {"Create Resource" , u8"자원 생성" });

		std::unique_ptr<EditorPopupNode> createMaterialNode =
			std::make_unique<EditorPopupNode>("JMaterial", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
		editorString->AddString(createMaterialNode->GetNodeId(), {"JMaterial" , u8"머테리얼" }); 

		std::unique_ptr<EditorPopupNode> createSceneNode =
			std::make_unique<EditorPopupNode>("JScene", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
		editorString->AddString(createSceneNode->GetNodeId(), {"JScene" , u8"씬" });

		std::unique_ptr<EditorPopupNode> createAnimationControllerNode =
			std::make_unique<EditorPopupNode>("AnimationContorller", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
		editorString->AddString(createAnimationControllerNode->GetNodeId(), {"AnimationContorller" , u8"애니메이션 컨트롤러" });

		std::unique_ptr<EditorPopupNode> eraseResourceNode =
			std::make_unique<EditorPopupNode>("EraseResource", EDITOR_POPUP_NODE_TYPE::INTERNAL, fileViewPopupRootNode.get());
		editorString->AddString(eraseResourceNode->GetNodeId(), { "Erase Resource" , u8"자원 삭제" });

		JDelegate<void(JDirectory*)> createMaterialFunc;
		createMaterialFunc.connect(this, &WindowDirectory::CreateMaterial);

		JDelegate<void(JDirectory*)> createSceneFunc;
		createSceneFunc.connect(this, &WindowDirectory::CreateScene);

		JDelegate<void(JDirectory*)> createAnimationControllerFunc;
		createAnimationControllerFunc.connect(this, &WindowDirectory::CreateAnimationController);
		 
		fileviewPopupFunc.emplace(createMaterialNode->GetNodeId(), createMaterialFunc);
		fileviewPopupFunc.emplace(createSceneNode->GetNodeId(), createSceneFunc);
		fileviewPopupFunc.emplace(createAnimationControllerNode->GetNodeId(), createAnimationControllerFunc);

		fileviewPopup = std::make_unique<EditorPopup>("Window JDirectory FileView Popup", std::move(fileViewPopupRootNode));
		fileviewPopup->AddPopupNode(std::move(createResourceNode));
		fileviewPopup->AddPopupNode(std::move(createMaterialNode));
		fileviewPopup->AddPopupNode(std::move(createSceneNode));
		fileviewPopup->AddPopupNode(std::move(createAnimationControllerNode));
		fileviewPopup->AddPopupNode(std::move(eraseResourceNode));

		editorPositionCal = std::make_unique<EditorObjectPositionCalculator>();
		previewGroup = JResourceManager::Instance().CreatePreviewGroup(GetName(), previewCapacity);
	}
	WindowDirectory::~WindowDirectory()
	{
	}
	void WindowDirectory::Initialize(EditorUtility* editorUtility)
	{
		const std::string contentPath = JApplicationVariable::GetProjectContentPath();
		root = JResourceManager::Instance().GetDirectory(contentPath);
		OpenNewDirectory(root);
	}
	void WindowDirectory::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		selectorIconMaxSize = editorUtility->displayWidth * selectorIconMaxRate;
		selectorIconMinSize = editorUtility->displayWidth * selectorIconMinRate;
		selectorIconSize = JMathHelper::Clamp<float>(selectorIconSize, selectorIconMinSize, selectorIconMaxSize);
		BuildDirectoryView(editorUtility);
	}
	bool WindowDirectory::Activate(EditorUtility* editorUtility)
	{
		if (Editor::Activate(editorUtility))
		{
			this->AddEventListener(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::MOUSE_CLICK);
			return true;
		}
		else
			return false;
	}
	bool WindowDirectory::DeActivate(EditorUtility* editorUtility)
	{
		if (Editor::DeActivate(editorUtility))
		{
			this->EraseListener(*editorUtility->EvInterface(), GetGuid());
			JResourceManager::Instance().ClearPreviewGroup(previewGroup);
			return true;
		}
		else
			return false;
	}
	bool WindowDirectory::OnFocus(EditorUtility* editorUtility)
	{
		if (Editor::OnFocus(editorUtility))
			return true;
		else
			return false;
	}
	bool WindowDirectory::OffFocus(EditorUtility* editorUtility)
	{
		if (Editor::OffFocus(editorUtility))
		{
			fileviewPopup->SetOpen(false);
			return true;
		}
		else
			return false;
	}
	void WindowDirectory::BuildDirectoryView(EditorUtility* editorUtility)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::Text(editorString->GetString(selectorIconSlidebarId).c_str());
		ImGui::SameLine();
		int sliderOffset = ImGui::GetStyle().ScrollbarSize + (ImGui::GetStyle().ItemSpacing.x * 2) + (editorUtility->textWidth * 4);
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - sliderOffset);
		ImGui::SliderFloat("##WindowDirectory_SizeSlider", &selectorIconSize, selectorIconMinSize, selectorIconMaxSize, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::BeginChild(directoryViewName.c_str(),
			ImVec2(windowSize.x * 0.2f, windowSize.y),
			true,
			ImGuiWindowFlags_AlwaysAutoResize);

		JDirectory* preOpend = opendDirctory;
		DirectoryViewOnScreen(root);
		SetTreeNodeDefaultColor();

		if (preOpend != opendDirctory)
			OpenNewDirectory(opendDirctory);

		ImGui::EndChild();
		ImGui::SameLine();
		BuildFileView(editorUtility);
	}
	void WindowDirectory::DirectoryViewOnScreen(JDirectory* directory)
	{
		bool isSelected = false;
		if (opendDirctory != nullptr && opendDirctory->GetGuid() == directory->GetGuid())
		{
			isSelected = true;
			SetTreeNodeSelectColor();
		}

		if (ImGui::TreeNodeEx(directory->GetName().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected))
		{
			if (isSelected)
				SetTreeNodeDefaultColor();

			if (ImGui::IsItemClicked())
				opendDirctory = directory;

			const uint childDirCount = directory->GetChildernDirctoryCount();
			for (uint i = 0; i < childDirCount; ++i)
			{
				JDirectory* child = directory->GetChildDirctory(i);
				if (child == nullptr)
					continue;
				DirectoryViewOnScreen(child);
			}
			ImGui::TreePop();
		}
	}
	void WindowDirectory::BuildFileView(EditorUtility* editorUtility)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::BeginChild(fileViewName.c_str(),
			ImVec2(windowSize.x * 0.8f, windowSize.y),
			true,
			ImGuiWindowFlags_None);

		FileViewOnScreen(editorUtility);

		if (fileviewPopup->IsOpen())
		{
			EDITOR_POPUP_NODE_RES res;
			size_t menuGuid;
			fileviewPopup->ExecutePopup(editorString.get(), res, menuGuid);
			if (res == EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
			{
				auto func = fileviewPopupFunc.find(menuGuid);
				if (func != fileviewPopupFunc.end())
					func->second.Invoke(opendDirctory);

				fileviewPopup->SetOpen(false);
			}
		}
		if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
		{
			if (editorUtility->rightMouseClick)
				fileviewPopup->SetOpen(!fileviewPopup->IsOpen());
			else if (editorUtility->leftMouseClick && fileviewPopup->IsOpen() && !fileviewPopup->IsMouseInPopup())
				fileviewPopup->SetOpen(false);
		}
		ImGui::EndChild();
	}
	void WindowDirectory::FileViewOnScreen(EditorUtility* editorUtility)
	{
		ImVec2 nowCursor = ImGui::GetCursorPos();
		ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
		float frameBorder = ImGui::GetStyle().FrameBorderSize;
		editorPositionCal->Update(ImGui::GetWindowWidth() * 0.75f, ImGui::GetWindowHeight(), selectorIconSize, selectorIconSize,
			nowCursor.x, nowCursor.y, itemSpacing.x, itemSpacing.y, frameBorder, editorUtility->textWidth, editorUtility->textHeight);

		//ImGuiManager::SetColor(ImVec4(1,1,1,0.1f), ImGuiCol_Header);   
		const uint count = previewGroup->GetPreviewSceneCount();
		for (uint i = 0; i < count; ++i)
		{
			PreviewScene* nowPreviewScene = previewGroup->GetPreviewScene(i);
			if (nowPreviewScene != nullptr)
			{
				JResourceObject* nowObject = nowPreviewScene->GetResouceObject();
				const JOBJECT_FLAG flag = nowObject->GetFlag();
				if ((flag & OBJECT_FLAG_HIDDEN) > 0)
					continue;

				std::string name = nowObject->GetName() + nowObject->GetFormat();
				const std::string path = nowObject->GetPath();
				const J_RESOURCE_TYPE resourceType = nowObject->GetResourceType();

				bool isSelected = false;
				if (editorUtility->selectedObject != nullptr && editorUtility->selectedObject->GetGuid() == nowObject->GetGuid())
				{
					isSelected = true;
					SetButtonSelectColor();
				}

				ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
				/*if (ImGui::ImageButton((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(nowPreviewScene->GetPreviewCamera(0)->GetRsSrvHeapIndex()).ptr,
					ImVec2(selectorIconSize, selectorIconSize)))
				{
					if (isSelected)
						SetButtonDefaultColor();

					editorUtility->selectedObject = nowObject;
					if (resourceType != J_RESOURCE_TYPE::NONE)
					{
						std::unique_ptr<EditorSelectResourceEvStruct> selectEvStruct =
							std::make_unique<EditorSelectResourceEvStruct>(nowObject);
						this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::SELECT_RESOURCE, selectEvStruct.get());
					}
					if (resourceType == J_RESOURCE_TYPE::SKELETON)
					{
						std::unique_ptr<EditorOpenPageEvStruct> openPageEvStruct =
							std::make_unique<EditorOpenPageEvStruct>("SkeletonAssetPage");
						this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::OPEN_PAGE, openPageEvStruct.get());
					}
				}
				*/
				if(isSelected)
					SetButtonDefaultColor();

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					editorUtility->selectedObject = nowObject;
					editorUtility->isDrag = true;
					int noUse = 0;
					std::string selectResourceName = nowObject->GetName();
					ImGui::Text(selectResourceName.c_str());
					ImGui::SetDragDropPayload(selectResourceName.c_str(), &noUse, sizeof(int));
					ImGui::EndDragDropSource();
				}

				ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(0)));
				int maxTextCount = editorPositionCal->GetSameLineMaxTextCount();
				int lineCount = 0;

				std::string subName;
				while (name.size() > maxTextCount)
				{
					++lineCount;
					subName = name.substr(maxTextCount, name.size());
					ImGui::Text(name.substr(0, maxTextCount).c_str());
					ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(lineCount)));
					name = subName;
				}
				ImGui::Text(name.c_str());
				editorPositionCal->Next();
			}
		}
		//ImGuiManager::ReturnDefaultColor(ImGuiCol_Header);
	}
	void WindowDirectory::OpenNewDirectory(JDirectory* newOpendDirectory)
	{
		const uint fileCount = (uint)newOpendDirectory->GetFileCount();
		JResourceManager::Instance().ClearPreviewGroup(previewGroup);

		for (uint i = 0; i < fileCount; ++i)
		{
			JFile* file = newOpendDirectory->GetFile(i);
			if (file == nullptr)
				continue;
			 
			PreviewScene* previewScene = JResourceManager::Instance().CreatePreviewScene(previewGroup, file->GetResource(), PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
		}
		if (opendDirctory != nullptr)
			opendDirctory->DirectoryInterface()->CloseDirectory();
		opendDirctory = newOpendDirectory;
		opendDirctory->DirectoryInterface()->OpenDirectory();
	}
	void WindowDirectory::CreateNewFolder(JDirectory* parent)noexcept
	{

	}
	void WindowDirectory::CreateMaterial(JDirectory* directory)
	{
		JMaterial* mat = JRFI<JMaterial>::Create(*directory);
		if (mat != nullptr)
		{
			JFile* addedFile = directory->GetFile(directory->GetFileCount() - 1);
			PreviewScene* previewScene = JResourceManager::Instance().CreatePreviewScene(previewGroup, mat, PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
		}
	}
	void WindowDirectory::CreateAnimationController(JDirectory* directory)
	{
		JAnimationController* aniController = JRFI<JAnimationController>::Create(*directory);
		if (aniController != nullptr)
		{
			JFile* addedFile = directory->GetFile(directory->GetFileCount() - 1);
			PreviewScene* previewScene = JResourceManager::Instance().CreatePreviewScene(previewGroup, aniController, PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
		}
	}
	//수정필요
	void WindowDirectory::CreateScene(JDirectory* directory)
	{
		JScene* scene = JRFI<JScene>::Create(*directory);
		if (scene != nullptr)
		{
			JFile* addedFile = directory->GetFile(directory->GetFileCount() - 1);
			PreviewScene* previewScene = JResourceManager::Instance().CreatePreviewScene(previewGroup, scene, PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
		}
	}
	void WindowDirectory::EraseDirectory(JDirectory* directory)noexcept
	{

	}
	void WindowDirectory::EraseFile(JFile* file)noexcept
	{

	}
	void WindowDirectory::OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)
	{
		if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
			return;
		if (eventType == EDITOR_EVENT::MOUSE_CLICK)
			fileviewPopup->SetOpen(false);
	}
}