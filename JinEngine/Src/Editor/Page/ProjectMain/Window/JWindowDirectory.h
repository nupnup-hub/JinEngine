#pragma once
#include"../../JEditorWindow.h"     
#include"../../WindowInterface/JEditorPreviewInterface.h"
#include"../../../Interface/JEditorObjectInterface.h"
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
		class JEditorStringMap;
		class JEditorPopupMenu; 
		class JEditorSearchBarHelper;
		class JEditorRenameHelper;

		class JWindowDirectory final : public JEditorWindow, 
			public JEditorPreviewInterface,
			public JEditorObjectHandlerInterface
		{ 
		private:
			//Create Resource & Directory
			using CreateObjectF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>, J_RESOURCE_TYPE>;
			using CreateDirectoryF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>>;
			using DestroyObjectF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JObject>>;
			using OpenNewDirectoryF = Core::JMFunctorType<JWindowDirectory, void, Core::JUserPtr<JDirectory>>;
			using ImportResourceF = Core::JMFunctorType<JWindowDirectory, void>;
			using CreateImportedResourceF = Core::JSFunctorType<void, JWindowDirectory*, std::vector<JResourceObject*>>;
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
			std::unique_ptr<JEditorStringMap> editorString;
			std::unique_ptr<JEditorRenameHelper> renameHelper;
			//not use
			//std::unique_ptr<JEditorPopupMenu>directoryViewPopup;
			std::unique_ptr<JEditorPopupMenu>fileviewPopup; 
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
		private:
			std::unique_ptr<CreateObjectF::Functor> createResourceF;
			std::unique_ptr<CreateDirectoryF::Functor> createDirectoryF;
			std::unique_ptr<DestroyObjectF::Functor> destroyObjectF;
			std::unique_ptr<OpenNewDirectoryF::Functor> openNewDirF;
			std::unique_ptr<OpenNewDirectoryF::CompletelyBind> openNewDirB;
			std::unique_ptr<ImportResourceF::Functor> importResourceF;
			std::unique_ptr<CreateImportedResourceF::Functor> createImportedResourceF;
			std::unique_ptr<RenameF::Functor> renameF;
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
			JVector2<float> fileTitleBarSize; 
			JVector2<float> renameCursorPos;
			JVector2<float> renameRectSize;
		private:
			float childWindowHeight = 0; 
			std::wstring importFilePath; 
		public:
			JWindowDirectory(const std::string &name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
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