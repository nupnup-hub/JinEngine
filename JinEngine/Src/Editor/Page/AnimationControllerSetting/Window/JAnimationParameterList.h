#pragma once
#include"../../JEditorWindow.h"   
#include"../../../../Core/FSM/JFSMparameterValueType.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{
		class JAnimationFSMdiagram;
		class JFSMparameter;
	}
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;

		class JAnimationParameterListCreationImpl;
		class JAnimationParameterListSettingImpl;
		class JAnimationParameterList final : public JEditorWindow
		{
		private:
			JUserPtr<JAnimationController> aniCont;
			JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram;  
		private:
			std::unique_ptr<JEditorPopupMenu> parameterListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			std::unique_ptr<JAnimationParameterListCreationImpl> creationImpl;
			std::unique_ptr<JAnimationParameterListSettingImpl> settingImpl;
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
			void Initialize(JUserPtr<JAnimationController> newAniCont)noexcept;
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
