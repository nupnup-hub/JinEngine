#pragma once
#include"../../JEditorWindow.h"     
#include"../../../Interface/JEditorObjectHandleInterface.h"

namespace JinEngine
{
	class JGameObject;
	class JResourceObject;
	class JDirectory;
	namespace Core
	{
		class Core::JIdentifier;
		class JFSMinterface;
	}
	namespace Editor
	{
		class JReflectionGuiWidgetHelper;
		class JEditorSearchBarHelper;

		class JObjectDetail final : public JEditorWindow,
			public JEditorObjectHandlerInterface
		{
		private:
			JUserPtr<Core::JIdentifier> selected;
		private:
			std::unique_ptr<JReflectionGuiWidgetHelper> guiHelper;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper; 
		public:
			JObjectDetail(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JObjectDetail();
			JObjectDetail(const JObjectDetail& rhs) = delete;
			JObjectDetail& operator=(const JObjectDetail& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void UpdateWindow()final;
		private:
			void BuildObjectDetail();
			void GameObjectDetailOnScreen(JUserPtr<JGameObject> gObj);
			void ObjectOnScreen(JUserPtr<Core::JIdentifier> fObj);
		private:
			void PropertyOnScreen(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo);
			void MethodOnScreen(Core::JIdentifier* obj, Core::JMethodInfo* mInfo);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)final;
		};
	}
}