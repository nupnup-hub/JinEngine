#pragma once
#include"../../JEditorWindow.h"   
#include"../../../Interface/JEditorObjectHandleInterface.h"
#include"../../../../Object/JObjectType.h"
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

		class JObjectExplorerCreationImpl;
		class JObjectExplorerSettingImpl;
		class JObjectExplorer final : public JEditorWindow, public JEditorObjectHandlerInterface
		{ 
		private:
			JUserPtr<JGameObject> root;   
			std::unique_ptr<JEditorStringMap>editorString;
			std::unique_ptr<JEditorRenameHelper>renameHelper;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
			std::unique_ptr<JEditorPopupMenu>explorerPopup; 
		private:
			std::unique_ptr<JObjectExplorerCreationImpl> creationImpl;
			std::unique_ptr<JObjectExplorerSettingImpl> settingImpl;
		public:
			JObjectExplorer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
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
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
			void DoSetUnFocus()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}