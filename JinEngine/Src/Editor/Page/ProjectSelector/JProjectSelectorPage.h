#pragma once
#include"../JEditorPage.h"
#include"../../../Object/Resource/JResourceUserInterface.h"

namespace JinEngine
{
	class JTexture;
	namespace Editor
	{
		class JProjectSelectorHub;
		class JProjectSelectorPage final: public JEditorPage,
			public JResourceUserInterface
		{ 
		private:
			std::unique_ptr< JProjectSelectorHub> projectHub;
			Core::JUserPtr<JTexture> backgroundTexture;
		public:
			JProjectSelectorPage();
			~JProjectSelectorPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public:
			//hasImguiTxt = imgui widget data .txt file
			void Initialize()final;
		public: 
			void UpdatePage(const JEditorPageUpdateCondition& condition)final;
		public:
			bool IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept final;
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}