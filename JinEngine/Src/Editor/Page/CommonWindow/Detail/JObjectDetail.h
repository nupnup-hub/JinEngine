#pragma once
#include"../../JEditorWindow.h"     
namespace JinEngine
{ 
	class JObject;
	class JGameObject;
	class JResourceObject;
	class JDirectory; 

	namespace Editor
	{
		class JReflectionGuiWidgetHelper;
		class JObjectDetail final : public JEditorWindow
		{ 
		private:
			std::unique_ptr<JReflectionGuiWidgetHelper> guiHelper;
			Core::JUserPtr<JObject> preSelected; 
		public:
			JObjectDetail(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
			~JObjectDetail();
			JObjectDetail(const JObjectDetail& rhs) = delete;
			JObjectDetail& operator=(const JObjectDetail& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public: 
			void UpdateWindow()final; 
		private:
			void BuildObjectDetail();
			void GameObjectDetailOnScreen(Core::JUserPtr<JGameObject> gObj);
			void ResourceObjectDetailOnScreen(Core::JUserPtr<JResourceObject> rObj);
			void DirectoryObjectDetailOnScreen(Core::JUserPtr<JDirectory> dObj);
		private:
			void PropertyOnScreen(JObject* obj, Core::JPropertyInfo* pInfo);
			void MethodOnScreen(JObject* obj, Core::JMethodInfo* mInfo);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}