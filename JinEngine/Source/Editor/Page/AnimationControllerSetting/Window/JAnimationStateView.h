#pragma once
#include"JEditorAniContInterface.h"
#include"../../JEditorWindow.h"   
#include"../../../String/JEditorStringMap.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../Popup/JEditorPopupMenu.h" 
#include"../../../../Core/Event/JEventListener.h" 

namespace JinEngine
{
	class JAnimationController;
	class JAnimationFSMdiagram;
	class JAnimationFSMstate;
	class JAnimationFSMtransition;
	namespace Editor
	{
		class JEditorGraphView;
		class JEditorPopupMenu;
		class JEditorStringMap;

		class JAnimationStateViewCreationFunctor;
		class JAnimationStateView final : public JEditorWindow, public JEditorAniContInterface
		{ 
		private:
			JUserPtr<JAnimationController> aniCont;
			JUserPtr<JAnimationFSMdiagram> selectedDiagram; 
		private:
			std::unique_ptr<JEditorGraphView> stateGraph;
			std::unique_ptr<JEditorPopupMenu> statePopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			std::unique_ptr<JAnimationStateViewCreationFunctor> creation;
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
			void SetAnimationController(const JUserPtr<JAnimationController>& newAniCont)final;
		private:
			void SetSelecteObject(JUserPtr<Core::JIdentifier> newSelected);
		public: 
			void UpdateWindow()final; 
		private:
			void BuildDiagramView();
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
