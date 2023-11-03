#pragma once
#include"../../JEditorWindow.h"   
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Core/Transition/JTransition.h"

namespace JinEngine
{
	class JGameObject;
	class JObject;
	class JMeshGeometry;
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorRenameHelper;
		class JEditorPopupMenu;
		class JEditorSearchBarHelper;
		class JEditorTreeStructure;

		class JObjectExplorerCreationFunctor;
		class JObjectExplorerSettingFunctor;
		class JObjectExplorer final : public JEditorWindow, public JObjectModifyInterface
		{ 
		private:
			JUserPtr<JGameObject> root;   
			std::unique_ptr<JEditorStringMap>editorString;
			std::unique_ptr<JEditorRenameHelper>renameHelper;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
			std::unique_ptr<JEditorPopupMenu>explorerPopup; 
			std::unique_ptr<JEditorTreeStructure> treeStrcture;
		private:
			std::unique_ptr<JObjectExplorerCreationFunctor> creation;
			std::unique_ptr<JObjectExplorerSettingFunctor> setting;
		public:
			JObjectExplorer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag,
				const std::vector<size_t>& listenWindowGuidVec);
			~JObjectExplorer();
			JObjectExplorer(const JObjectExplorer& rhs) = delete;
			JObjectExplorer& operator=(const JObjectExplorer& rhs) = delete;
		private:
			void InitializeCreationImpl();
			void InitializeSettingImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(JUserPtr<JGameObject> newRoot)noexcept;
			void UpdateWindow()final;
		private:
			void BuildObjectExplorer();
			void ObjectExplorerOnScreen(const JUserPtr<JGameObject>& gObj, const bool isAcivatedSearch);
		private:
			void DisplayActSignalWidget(const JUserPtr<JGameObject>& gObj, const bool allowDisplaySeletable);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
			void DoSetUnFocus()noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}