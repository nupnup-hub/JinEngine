#pragma once
#include"../EditorWindow.h"   
#include"../../Event/EditorEventType.h"
#include"../../Event/EditorEventStruct.h" 
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Utility/JDelegate.h"
#include<stdlib.h> 
#include<locale.h> 
#include<vector>
#include<unordered_map>

namespace JinEngine
{ 
	class JDirectory;
	class EditorString;
	class EditorPopup;
	class EditorObjectPositionCalculator;
	class JFile;
	class PreviewSceneGroup;

	class WindowDirectory : public EditorWindow, public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{
	private:  
		const std::string directoryViewName = "DirectoryView";
		const std::string fileViewName = "FileView"; 
		JDirectory* root;
		JDirectory* opendDirctory;  

		std::unique_ptr<EditorString> editorString;
		std::unique_ptr<EditorPopup>fileviewPopup; 
		std::unordered_map<size_t, JDelegate<void(JDirectory*)>> fileviewPopupFunc;
		std::unique_ptr<EditorObjectPositionCalculator> editorPositionCal;
		static constexpr float selectorIconMaxRate = 0.075f;
		static constexpr float selectorIconMinRate = 0.035f;
		float selectorIconMinSize;
		float selectorIconMaxSize;
		float selectorIconSize = 0;
		size_t selectorIconSlidebarId;

		static constexpr int previewCapacity = 100;
		const size_t previewGuid;
		PreviewSceneGroup* previewGroup;
	public:
		WindowDirectory(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~WindowDirectory();
		WindowDirectory(const WindowDirectory& rhs) = delete;
		WindowDirectory& operator=(const WindowDirectory& rhs) = delete;

		void Initialize( EditorUtility* editorUtility);
		void UpdateWindow(EditorUtility* editorUtility)override;
		 
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
		bool OnFocus(EditorUtility* editorUtility) final;
		bool OffFocus(EditorUtility* editorUtility) final;
	private:  
		void BuildDirectoryView(EditorUtility* editorUtility);
		void BuildFileView(EditorUtility* editorUtility);
		void DirectoryViewOnScreen(JDirectory* directory);
		void FileViewOnScreen(EditorUtility* editorUtility); 
		 
		void OpenNewDirectory(JDirectory* newOpendDirectory);
		void CreateNewFolder(JDirectory* parent)noexcept;
		void CreateMaterial(JDirectory* directory);
		void CreateAnimationController(JDirectory* directory);
		void CreateScene(JDirectory* directory);
		void EraseDirectory(JDirectory* directory)noexcept;
		void EraseFile(JFile* file)noexcept;
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)final;
	};
}
