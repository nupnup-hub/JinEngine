#pragma once
#include"../../JEditorWindow.h"     
#include"../../WindowInterface/JEditorPreviewInterface.h" 
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/JResourceObjectType.h" 
#include<stdlib.h> 
#include<locale.h> 
#include<vector>
#include<unordered_map>
 
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
		class JEditorSearchBarHelper;
		class JEditorRenameHelper;
		class JEditorTreeStructure;

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
		private:
			JUserPtr<JDirectory> root;
			JUserPtr<JDirectory> opendDirctory; 
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
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
			std::unique_ptr<JEditorTreeStructure> treeStrcture;
		private:
			ImportData importData;
		private:
			static constexpr float selectorIconMaxRate = 0.075f;
			static constexpr float selectorIconMinRate = 0.035f;
			float btnIconMaxSize;
			float btnIconMinSize;
			float btnIconSize = 0;
			size_t selectorIconSlidebarId; 
			JVector2<float> fileTitleBarSize; 
			JVector2<float> renameCursorPos;
			JVector2<float> renameRectSize;
		private:
			float childWindowHeight = 0;  
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
			void CreateDirectoryPreview(const JUserPtr<JDirectory>& directory, const bool hasNameMask, const std::wstring& mask = L"");
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