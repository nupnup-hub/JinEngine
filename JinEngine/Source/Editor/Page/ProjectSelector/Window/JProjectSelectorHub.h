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
#include"../../JEditorWindow.h"  
#include"../../../Interface/JEditorProjectInterface.h"
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/Resource/JResourceObjectEventType.h"
#include"../../../../Object/Resource/JResourceObject.h" 

namespace JinEngine
{
	class JTexture; 
	namespace Editor
	{
		struct SelectorValues;
		class JEditorSearchBar;
		class JEditorDynamicSpotColor;
		class JProjectSelectorHubCreationFunctor;
		class JProjectSelectorHub : public JEditorWindow, public JResourceEventManager::Listener
		{ 
		private:
			using ResourceEvListener = JResourceEventManager::Listener;
		private:
			std::unique_ptr<JProjectSelectorHubCreationFunctor> creation;
		private:
			std::unique_ptr<JEditorProjectInterface> pInterface;
		private: 
			JUserPtr<JTexture> serachIconTexture;
			JUserPtr<JTexture> optionSettingTexture;
			JUserPtr<JTexture> backgroundTexture;
		private:
			//last project rendering scene
			std::vector<JUserPtr<JTexture>> lastRSVec;
			std::unique_ptr<JEditorSearchBar> searchHelper;
			std::unique_ptr<JEditorDynamicSpotColor> dynamicCol;
		private:
			std::unique_ptr<SelectorValues> values;
			float optionListCusorY;
			static constexpr uint necessaryCapacityMB = 200;
		private:
			int newProjVersion; 
			int versionIndex = 0;
		public:
			JProjectSelectorHub(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag,
				std::unique_ptr<JEditorProjectInterface>&& newPInterface);
			~JProjectSelectorHub();
			JProjectSelectorHub(const JProjectSelectorHub& rhs) = delete;
			JProjectSelectorHub& operator=(const JProjectSelectorHub& rhs) = delete;
		private:
			void InitializeCreationImpl(); 
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public: 
			void UpdateWindow()final;
		private:
			void UpdateCanvasSize();
		private:
			void TitleOnScreen();
			void ProjectListOnScreen(); 
			void ProjectDetailOnScreen();
			void GuideButtonOnScreen();
			void CreateNewProjectOnScreen();
			void LoadProjectOnScreen();
			void OptionOnScreen();
		private:
			void RequestDestroyProject(); 
		private:
			void CreateProjectProccess();
		private:
			void LoadLastRsTexture();
		private:
			void DoSetOpen()noexcept final;
			void DoSetClose()noexcept final; 
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)final;
		};
	}
}