/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../JEditorPage.h" 
#include"../../../Object/Resource/JResourceObjectEventType.h"
#include"../../../Object/Resource/JResourceObject.h"

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
		class JEditorSkeletonPage final: public JEditorPage, public JResourceEventManager::Listener
		{
		private:
			using ResourceEvListener = JResourceEventManager::Listener;
		private:
			std::unique_ptr< JObjectExplorer> explorer;
			std::unique_ptr< JAvatarEditor> avatarEdit;
			std::unique_ptr< JSceneObserver> avatarObserver;
			std::unique_ptr<JObjectDetail>avatarDetail; 
		private:
			std::unique_ptr<JEditorMenuBar> menuBar;
		private:
			JUserPtr<JSkeletonAsset> skeleotnAsset;
			JUserPtr<JScene> avatarScene;
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
			bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			bool StuffSkeletonAssetData(const JUserPtr<Core::JIdentifier>& selectedObj);
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept;
			void DoDeActivate()noexcept;
		private:
			void LoadPage(JFileIOTool& tool)final;
			void StorePage(JFileIOTool& tool)final;
		private:
			void BuildDockNode();
			void BuildMenuNode();
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)final;
		};
	}
}
