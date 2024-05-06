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
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Core/Transition/JTransition.h"

namespace JinEngine
{
	class JGameObject;
	class JObject;
	class JMeshGeometry;
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorRenameHelper;
		class JEditorPopupMenu;
		class JEditorSearchBar;
		class JEditorTreeStructure;

		class JObjectExplorerCreationFunctor;
		class JObjectExplorerSettingFunctor;
		class JObjectExplorer final : public JEditorWindow, public JObjectModifyInterface
		{ 
		private:
			JUserPtr<JGameObject> root;   
			std::unique_ptr<JEditorStringMap>editorString;
			std::unique_ptr<JEditorRenameHelper>renameHelper;
			std::unique_ptr<JEditorSearchBar> searchBarHelper;
			std::unique_ptr<JEditorPopupMenu>explorerPopup; 
			std::unique_ptr<JEditorTreeStructure> treeStrcture;
		private:
			std::unique_ptr<JObjectExplorerCreationFunctor> creation;
			std::unique_ptr<JObjectExplorerSettingFunctor> setting;
		private:
			bool isFocusedThisFrame = false;
		public:
			JObjectExplorer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag,
				const std::vector<size_t>& listenWindowGuidVec);
			~JObjectExplorer();
			JObjectExplorer(const JObjectExplorer& rhs) = delete;
			JObjectExplorer& operator=(const JObjectExplorer& rhs) = delete;
		private:
			void InitializeCreationImpl();
			void InitializeSettingImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(JUserPtr<JGameObject> newRoot)noexcept;
			void UpdateWindow()final;
		private:
			void BuildObjectExplorer();
			void ObjectExplorerOnScreen(const JUserPtr<JGameObject>& gObj, const bool isAcivatedSearch);
		private:
			void DisplayActSignalWidget(const JUserPtr<JGameObject>& gObj, const bool allowDisplaySeletable);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
			void DoSetUnFocus()noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}