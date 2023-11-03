#pragma once
#include"../JEditorPage.h"
#include"../../Interface/JEditorProjectInterface.h"
#include"../../../Core/Event/JEventListener.h" 
#include"../../../Object/Resource/JResourceObjectEventType.h"
#include"../../../Object/Resource/JResourceObject.h"

namespace JinEngine
{ 
	class JTexture;
	struct JResourceEventDesc;
	namespace Editor
	{
		class JProjectSelectorHub; 
		class JProjectSelectorPage final: public JEditorPage, public JResourceEventManager::Listener
		{
		private:
			using ResourceEvListener = JResourceEventManager::Listener;
		private:
			std::unique_ptr<JProjectSelectorHub> projectHub;
			JUserPtr<JTexture> backgroundTexture; 
		public:
			JProjectSelectorPage(std::unique_ptr< JEditorProjectInterface>&& pInterface);
			~JProjectSelectorPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public: 
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)final;
		};
	}
}