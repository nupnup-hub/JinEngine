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


#pragma once
#include"../../JEditorWindow.h"     
#include"../../WindowInterface/JEditorPreviewInterface.h" 
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/JResourceObjectType.h" 
#include<stdlib.h> 
#include<locale.h>  
 
namespace JinEngine
{
	class JFile;
	class JDirectory; 
	class JResourceObject;
	class JObject;
	class JResourceObjectImportDesc;

	namespace Editor
	{
		class JEditorStringMap;
		class JEditorPopupMenu; 
		class JEditorSearchBar;
		class JEditorAddressBar;
		class JEditorPageCounter;
		class JEditorRenameHelper;
		class JEditorTreeStructure;
		class JEditorSelectableStructure;
		class JEditorMouseDragCanvas;

		class JWindowDirectoryCreationFunctor;
		class JWindowDirectorySettingFunctor;

		class JWindowDirectory final : public JEditorWindow,
			public JEditorPreviewInterface,
			public JObjectModifyInterface
		{ 
		private:
			struct ImportData
			{
			public:
				std::vector<std::unique_ptr<JResourceObjectImportDesc>> importDesc;
				std::vector<J_RESOURCE_TYPE> importRType;
				JUserPtr<JDirectory> folder; 
				int descIndex = 0;
				bool isActivatedImportWindow = false; 
			};
			struct IconData
			{
			public:
				float maxSize;
				float minSize;
				float size = 0;
				bool isDetail = true;
			};
		private:
			JUserPtr<JDirectory> root; 
			bool lastUpdateOpenNewDir = false; 
		private:
			std::unique_ptr<JWindowDirectoryCreationFunctor> creation;
			std::unique_ptr<JWindowDirectorySettingFunctor> setting;
		private:
			std::unique_ptr<JEditorStringMap> editorString;
			std::unique_ptr<JEditorRenameHelper> renameHelper;
			//not use
			//std::unique_ptr<JEditorPopupMenu>directoryViewPopup;
			std::unique_ptr<JEditorPopupMenu>fileviewPopup; 
			std::unique_ptr<JEditorSearchBar> searchBar;
			std::unique_ptr<JEditorAddressBar> addressBar;
			std::unique_ptr<JEditorPageCounter> pageCounter;
			std::unique_ptr<JEditorTreeStructure> treeStrcture;
			std::unique_ptr<JEditorSelectableStructure> selectableStructure;
			std::unique_ptr<JEditorMouseDragCanvas> mouseBBox;
		private:
			ImportData importData;
			IconData iconData;
		private: 
			size_t selectorIconSlidebarId; 
			JVector2<float> fileTitleBarSize; 
			JVector2<float> renameCursorPos;
			JVector2<float> renameRectSize;
		private:
			float childWindowHeight = 0;  
			bool requestUpdatePreview = false;
		public:
			JWindowDirectory(const std::string &name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JWindowDirectory();
			JWindowDirectory(const JWindowDirectory& rhs) = delete;
			JWindowDirectory& operator=(const JWindowDirectory& rhs) = delete;
		private:
			void InitializeCreationImpl();
			void InitializeSettingImpl();
		private:
			void BuildPopup(); 
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		private:
			void SetImportSetting(const bool value)noexcept;
			void SwitchImportSetting()noexcept;
		public:
			void Initialize();
			void UpdateWindow()final;
		private:
			void BuildDirectoryView();
			void BuildFileView();
			//Ret is NewOpend Directory
			void DirectoryViewOnScreen(const JUserPtr<JDirectory>& directory, const bool canSelect);
			void FileViewOnScreen();  
			void ImportSettingOnScreen(); 
		private:
			void OpenNewDirectory(JUserPtr<JDirectory> newOpendDirectory);
			void CreateDirectoryPreview();
			//Only create file preview not directory
			void CreateAllDirectoryPreview(const JUserPtr<JDirectory>& directory, const bool hasNameMask, const std::wstring& mask = L"");
		private:
			void RequestMoveFile(const JUserPtr<JDirectory>& to, const JUserPtr<JObject>& obj);
			void MoveFile(JUserPtr<JDirectory> to, JUserPtr<JObject> obj);
		private:
			void RequestOpenNewDirectory(JUserPtr<JDirectory> newOpendDirectory);
			void RequestImportPostProccess(std::vector<JUserPtr<JResourceObject>>&& rVec);
		private:
			void DoSetFocus()noexcept final;
			void DoSetUnFocus()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool);
			void StoreEditorWindow(JFileIOTool& tool);
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}