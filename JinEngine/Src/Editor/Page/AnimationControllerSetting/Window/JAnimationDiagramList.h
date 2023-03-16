#pragma once
#include"../../JEditorWindow.h"   
#include"../../../../Core/FSM/JFSMparameterValueType.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JAnimationDiagramListCreationImpl;
		class JAnimationDiagramList final : public JEditorWindow
		{
		private:
			Core::JUserPtr<JAnimationController> aniCont;  
		private:
			std::unique_ptr<JEditorPopupMenu> diagramListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			std::unique_ptr<JAnimationDiagramListCreationImpl> creationImpl;
		public:
			JAnimationDiagramList(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationDiagramList();
			JAnimationDiagramList(const JAnimationDiagramList& rhs) = delete;
			JAnimationDiagramList& operator=(const JAnimationDiagramList& rhs) = delete;
		private:
			void InitializeCreationImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept;
			void UpdateWindow()final;
		private:
			void BuildDiagramList();
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev)final;
		};
	}
}
