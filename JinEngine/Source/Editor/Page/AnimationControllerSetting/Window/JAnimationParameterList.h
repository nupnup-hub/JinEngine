#pragma once
#include"JEditorAniContInterface.h"
#include"../../JEditorWindow.h"   
#include"../../../../Core/FSM/JFSMparameterValueType.h"

namespace JinEngine
{
	class JAnimationController;
	class JAnimationFSMdiagram;
	namespace Core
	{
		class JFSMparameter;
	}
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;

		class JAnimationParameterListCreationFunctor;
		class JAnimationParameterListSettingFunctor;
		class JAnimationParameterList final : public JEditorWindow, public JEditorAniContInterface
		{
		private:
			JUserPtr<JAnimationController> aniCont;
			JUserPtr<JAnimationFSMdiagram> selectedDiagram;  
		private:
			std::unique_ptr<JEditorPopupMenu> parameterListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			std::unique_ptr<JAnimationParameterListCreationFunctor> creation;
			std::unique_ptr<JAnimationParameterListSettingFunctor> setting;
		public:
			JAnimationParameterList(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationParameterList();
			JAnimationParameterList(const JAnimationParameterList& rhs) = delete;
			JAnimationParameterList& operator=(const JAnimationParameterList& rhs) = delete;
		private:
			void InitializeCreationImpl();
			void InitializeSettingImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void SetAnimationController(const JUserPtr<JAnimationController>& newAniCont)final;
		public: 
			void UpdateWindow()final;
		private:
			void BuildParameterList();
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev)final;
		};
	}
}
