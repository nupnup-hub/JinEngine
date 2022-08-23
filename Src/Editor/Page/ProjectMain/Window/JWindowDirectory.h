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
	namespace Editor
	{
		class JEditorString;
		class JEditorPopup;
		class JEditorWidgetPosCalculator;

		class JWindowDirectory : public JEditorWindow, public JEditorPreviewInterface
		{
		private:
			//Create Resource & Directory
			using CreateObjectFunctor = Core::JFunctor<void, JWindowDirectory&, JDirectory&>;
			using CreateObjectBinder = Core::JBindHandle<CreateObjectFunctor, JWindowDirectory&, JDirectory&>;

			using DestroyObjectFunctor = Core::JFunctor<void, JWindowDirectory&, JObject&>;
			using DestroyObjectBinder = Core::JBindHandle<DestroyObjectFunctor, JWindowDirectory&, JObject&>;

			using DestroyRecentFunctor = Core::JFunctor<void, JWindowDirectory&, JDirectory&>;
			using DestroyRecentBinder = Core::JBindHandle<DestroyRecentFunctor, JWindowDirectory&, JDirectory&>;

			using OpenNewDirectoryFunctor = Core::JFunctor<void, JWindowDirectory&, const size_t>;
			using OpenNewDirectoryBinder = Core::JBindHandle<OpenNewDirectoryFunctor, JWindowDirectory&, const size_t>;
		private:
			const std::string directoryViewName = "DirectoryView";
			const std::string fileViewName = "FileView";
			JDirectory* root;
			JDirectory* opendDirctory;

			std::unique_ptr<JEditorString> editorString;
			std::unique_ptr<JEditorPopup>fileviewPopup;
			std::unordered_map<size_t, CreateObjectFunctor> createResourceFuncMap;
			std::unordered_map<size_t, DestroyObjectFunctor> destroyResourceFuncMap;
			std::unique_ptr<DestroyRecentFunctor> destroyRecentFunc; 
			std::unique_ptr<OpenNewDirectoryFunctor> openNewDirectoryFunc;
			std::unique_ptr<OpenNewDirectoryBinder> openNewDirectoryBinder;

			std::unique_ptr<JEditorWidgetPosCalculator> editorPositionCal;
			static constexpr float selectorIconMaxRate = 0.075f;
			static constexpr float selectorIconMinRate = 0.035f;
			float btnIconMaxSize;
			float btnIconMinSize;
			float btnIconSize = 0;
			size_t selectorIconSlidebarId;
		private:
			Core::JFunctor<void, const size_t> openNewDirByGuidFunctor; 
		public:
			JWindowDirectory(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JWindowDirectory();
			JWindowDirectory(const JWindowDirectory& rhs) = delete;
			JWindowDirectory& operator=(const JWindowDirectory& rhs) = delete;
		public:
			void Initialize();
			void UpdateWindow()final;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
		private:
			void BuildDirectoryView();
			void DirectoryViewOnScreen(JDirectory* directory);
			void BuildFileView();
			void FileViewOnScreen();
			void ResourceFileViewOnScreen(JPreviewScene* nowPreviewScene, JResourceObject* jRobj);
			void DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir);
		private:
			void OpenNewDirectory(JDirectory* newOpendDirectory);
			void OpenNewDirectoryByGuid(const size_t guid); 
		protected:
			void DoSetFocus()noexcept final;
			void DoOffFocus()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)final;
		};
	}
}