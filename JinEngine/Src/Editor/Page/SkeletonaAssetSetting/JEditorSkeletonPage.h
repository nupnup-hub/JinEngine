#pragma once
#include"../JEditorPage.h" 
#include"../../../Object/Resource/JResourceUserInterface.h"

namespace JinEngine
{ 
	class JScene;
	class JSkeletonAsset;
	namespace Editor
	{
		class JObjectExplorer;
		class JAvatarEditor;  
		class JSceneObserver;
		class JObjectDetail;
		class JEditorMenuBar;
		class JEditorSkeletonPage final: public JEditorPage, public JResourceUserInterface
		{  
		private:
			std::unique_ptr< JObjectExplorer> explorer;
			std::unique_ptr< JAvatarEditor> avatarEdit;
			std::unique_ptr< JSceneObserver> avatarObserver;
			std::unique_ptr<JObjectDetail>avatarDetail;
		private:
			std::unique_ptr<JEditorMenuBar> menuBar;
		private:
			Core::JUserPtr<JSkeletonAsset> skeleotnAsset;
			Core::JUserPtr<JScene> avatarScene;
		private:
			bool setWndOptionOnce = false;
		public:
			JEditorSkeletonPage();
			~JEditorSkeletonPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public:
			void Initialize()final;
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const Core::JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			bool StuffSkeletonAssetData(const Core::JUserPtr<Core::JIdentifier>& selectedObj);
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept;
			void DoDeActivate()noexcept;
		public:
			void StorePage(std::wofstream& stream)final;
			void LoadPage(std::wifstream& stream)final;
		private:
			void BuildDockNode();
			void BuildMenuNode();
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}
