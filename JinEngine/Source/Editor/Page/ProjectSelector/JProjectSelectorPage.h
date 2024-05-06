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