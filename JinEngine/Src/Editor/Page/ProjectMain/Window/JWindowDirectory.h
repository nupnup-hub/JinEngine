#pragma once
#include"../../JEditorWindow.h"     
#include"../../WindowInterface/JEditorPreviewInterface.h"
#include"../../../Utility/JEditorRenameHelper.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/JResourceType.h"
#include"../../../../Core/Func/Callable/JCallable.h"
#include"../../../../Core/Func/Functor/JFunctor.h"
#include<stdlib.h> 
#include<locale.h> 
#include<vector>
#include<unordered_map>

struct ImGuiWindow;

namespace JinEngine
{
	class JFile;
	class JDirectory; 
	class JResourceObject;
	class JObject;

	namespace Editor
	{
		class JEditorString;
		class JEditorPopupMenu;
		class JEditorWidgetPosCalculator;
		class JEditorSearchBarHelper;

		class JWindowDirectory final : public JEditorWindow, public JEditorPreviewInterface
		{ 
		private:
			//Create Resource & Directory
			using CreateObjectF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>, J_RESOURCE_TYPE>;
			using CreateDirectoryF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>>;
			using DestroyObjectF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JObject>>;
			using OpenNewDirectoryF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>>;
			using ImportResourceF = Core::JMFunctorType<JWindowDirectory, void>;
			using RenameF = Core::JSFunctorType<void, JWindowDirectory*>;

			using RegisterCreateREvF = Core::JMFunctorType<JWindowDirectory, void, J_RESOURCE_TYPE>;
			using RegisterCreateDEvF = Core::JMFunctorType<JWindowDirectory, void>;
			using RegisterDestroyEvF = Core::JMFunctorType<JWindowDirectory, void>;
		private:
			Core::JUserPtr<JDirectory> root;
			Core::JUserPtr<JDirectory> opendDirctory;
			Core::JUserPtr<JObject> selectedObj;  
			bool lastUpdateOpenNewDir = false;
		private:
			std::unique_ptr<JEditorString> editorString;
			std::unique_ptr<JEditorRenameHelper> renameHelper;
			//not use
			//std::unique_ptr<JEditorPopupMenu>directoryViewPopup;
			std::unique_ptr<JEditorPopupMenu>fileviewPopup;
			std::unique_ptr<JEditorWidgetPosCalculator> editorPositionCal;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
		private:
			std::unique_ptr<CreateObjectF::Functor> createResourceFunctor;
			std::unique_ptr<CreateDirectoryF::Functor> createDirectoryFunctor;
			std::unique_ptr<DestroyObjectF::Functor> destroyObjectFunctor;
			std::unique_ptr<OpenNewDirectoryF::Functor> openNewDirFunctor;
			std::unique_ptr<OpenNewDirectoryF::CompletelyBind> openNewDirBinder;
			std::unique_ptr<ImportResourceF::Functor> importResourceFunctor;
			std::unique_ptr<RenameF::Functor> renameFunctor;
			std::unique_ptr<RegisterCreateREvF::Functor> regCreateRobjF;
			std::unique_ptr<RegisterCreateDEvF::Functor> regCreateDirF;
			std::unique_ptr<RegisterDestroyEvF::Functor> regDestroyObjF;
		private:
			static constexpr float selectorIconMaxRate = 0.075f;
			static constexpr float selectorIconMinRate = 0.035f;
			float btnIconMaxSize;
			float btnIconMinSize;
			float btnIconSize = 0;
			size_t selectorIconSlidebarId; 
		private:
			float childWindowHeight = 0;
		private:
			std::wstring importFilePath;
			bool actImport = false;
		public:
			JWindowDirectory(const std::string &name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JWindowDirectory();
			JWindowDirectory(const JWindowDirectory& rhs) = delete;
			JWindowDirectory& operator=(const JWindowDirectory& rhs) = delete;
		private:
			void BuildPopup(); 
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize();
			void UpdateWindow()final;
		private:
			void BuildDirectoryView();
			void BuildFileView();
			//Ret is NewOpend Directory
			JDirectory* DirectoryViewOnScreen(JDirectory* directory, const bool canSelect);
			void FileViewOnScreen(); 
			void ResourceFileViewOnScreen(JPreviewScene* nowPreviewScene, JResourceObject* jRobj);
			void DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir);
			void ImportFile(); 
		private:
			void OpenNewDirectory(Core::JUserPtr<JDirectory> newOpendDirectory);
			void CreateOpendDirectoryPreview(JDirectory* directory, const bool hasNameMask, const std::wstring& mask = L"");
			//Only create file preview not directory
			void CreateAllDirectoryPreview(JDirectory* directory, const bool hasNameMask, const std::wstring& mask = L"");
		private:
			void RegisterCreateResourceObjectEv(J_RESOURCE_TYPE shapeType);
			void RegisterCreateDirectoryEv();
			void RegisterDestroyResourceObjectEv();
			void CreateResourceObject(Core::JUserPtr<JDirectory> owner, const J_RESOURCE_TYPE rType);
			void CreateDirectory(Core::JUserPtr<JDirectory> parent);
			void DestroyObject(Core::JUserPtr<JObject> obj);
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