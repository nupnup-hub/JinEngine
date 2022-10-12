#pragma once
#include"../../JEditorWindow.h"     
#include"../../WindowInterface/JEditorPreviewInterface.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/JResourceType.h"
#include"../../../../Core/Func/Callable/JCallable.h"
#include"../../../../Core/Func/Functor/JFunctor.h"
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

	namespace Editor
	{
		class JEditorString;
		class JEditorPopup;
		class JEditorWidgetPosCalculator;

		class JWindowDirectory final : public JEditorWindow, public JEditorPreviewInterface
		{ 
		private:
			//Create Resource & Directory
			using CreateObjectF = Core::JFunctor<void, JWindowDirectory&, Core::JUserPtr<JDirectory>>;
			using DestroyObjectF = Core::JFunctor<void, JWindowDirectory&, Core::JUserPtr<JObject>>;
			 
			using OpenNewDirectoryF = Core::JFunctor<void, JWindowDirectory&, Core::JUserPtr<JDirectory>>;
			using OpenNewDirectoryB = Core::JBindHandle<OpenNewDirectoryF, JWindowDirectory&, Core::JUserPtr<JDirectory>>;
		
			using ImportResourceF = Core::JFunctor<void, JWindowDirectory&>;
		private:
			const std::string directoryViewName = "DirectoryView";
			const std::string fileViewName = "FileView";
			Core::JUserPtr<JDirectory> root;
			Core::JUserPtr<JDirectory> opendDirctory;

			std::unique_ptr<JEditorString> editorString;
			std::unique_ptr<JEditorPopup>fileviewPopup;
			std::unordered_map<size_t, std::unique_ptr<CreateObjectF>> createResourceFuncMap;
			std::unordered_map<size_t, std::unique_ptr<DestroyObjectF>> destroyResourceFuncMap;
			std::unique_ptr<OpenNewDirectoryF> openNewDirFunctor;
			std::unique_ptr<OpenNewDirectoryB> openNewDirBinder;
			std::tuple<size_t, std::unique_ptr<ImportResourceF>> importResourceT;

			std::unique_ptr<JEditorWidgetPosCalculator> editorPositionCal;
			static constexpr float selectorIconMaxRate = 0.075f;
			static constexpr float selectorIconMinRate = 0.035f;
			float btnIconMaxSize;
			float btnIconMinSize;
			float btnIconSize = 0;
			size_t selectorIconSlidebarId; 

			std::wstring importFilePath;
			bool actImport = false;
		public:
			JWindowDirectory(const std::string &name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JWindowDirectory();
			JWindowDirectory(const JWindowDirectory& rhs) = delete;
			JWindowDirectory& operator=(const JWindowDirectory& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize();
			void UpdateWindow()final;
		private:
			void BuildDirectoryView();
			void BuildFileView();
			//Ret is NewOpend Directory
			JDirectory* DirectoryViewOnScreen(JDirectory* directory);
			void FileViewOnScreen();
			void ResourceFileViewOnScreen(JPreviewScene* nowPreviewScene, JResourceObject* jRobj);
			void DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir);
			void ImportFile();
		private:
			void OpenNewDirectory(JDirectory* newOpendDirectory); 
		protected:
			void DoSetFocus()noexcept final;
			void DoSetUnFocus()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}