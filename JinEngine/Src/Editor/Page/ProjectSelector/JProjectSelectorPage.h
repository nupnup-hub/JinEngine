#pragma once
#include"../JEditorPage.h"
#include"../../../Core/Event/JEventListener.h" 
#include"../../../Object/Resource/JResourceObjectEventType.h"

namespace JinEngine
{
	class JResourceObject;
	class JTexture;
	namespace Editor
	{
		class JProjectSelectorHub; 
		class JProjectSelectorPage final: public JEditorPage, public Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
		{
		private:
			using ResourceEvListener = Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>;
		private:
			std::unique_ptr<JProjectSelectorHub> projectHub;
			Core::JUserPtr<JTexture> backgroundTexture; 
		public:
			JProjectSelectorPage();
			~JProjectSelectorPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public: 
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const Core::JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}