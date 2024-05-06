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
#include"../../../../Object/JObjectModifyInterface.h"

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
		class JEditorSearchBar;

		class JObjectDetail final : public JEditorWindow, public JObjectModifyInterface
		{
		private:
			JUserPtr<Core::JIdentifier> selected;
		private:
			std::unique_ptr<JReflectionGuiWidgetHelper> guiHelper;
			std::unique_ptr<JEditorSearchBar> searchBarHelper; 
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