#pragma once
#include"../../JEditorWindow.h"   
#include"../../../../Core/Event/JEventListener.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../Popup/JEditorPopupMenu.h" 

namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{ 
		class JAnimationFSMdiagram; 
		class JAnimationFSMstate;
		class JAnimationFSMtransition;
	}

	namespace Editor
	{
		class JEditorGraphView;
		class JEditorPopupMenu;
		class JEditorStringMap;

		class JAnimationStateViewCreationImpl;
		class JAnimationStateView final : public JEditorWindow
		{ 
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram; 
		private:
			std::unique_ptr<JEditorGraphView> stateGraph;
			std::unique_ptr<JEditorPopupMenu> statePopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			std::unique_ptr<JAnimationStateViewCreationImpl> creationImpl;
		public:
			JAnimationStateView(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationStateView();
			JAnimationStateView(const JAnimationStateView& rhs) = delete;
			JAnimationStateView& operator=(const JAnimationStateView& rhs) = delete;
		private:
			void InitializeCreationImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> aniCont)noexcept;
			void UpdateWindow()final; 
		private:
			void BuildDiagramView();
		private:
			void SetSelecteObject(Core::JUserPtr<Core::JIdentifier> newSelected); 
		private:
			void RegisterViewGraphGroup(JAnimationController* newAnicont)noexcept;
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
		};
	}
}
